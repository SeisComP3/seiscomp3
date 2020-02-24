from __future__ import print_function
import os, string, time, re, glob, shutil, sys, imp, random, fnmatch
from seiscomp3 import Core, Kernel, Config, System, Client, Communication, DataModel

DEBUG = 0

'''
NOTE:
All parameters from arclink.cfg are not prefixed with "arclink.".
Local parameters that are created from arclink.cfg parameters are
prefixed with "arclink.".
'''

def collectParams(container):
    params = {}

    for i in range(container.groupCount()):
        params.update(collectParams(container.group(i)))

    for i in range(container.structureCount()):
        params.update(collectParams(container.structure(i)))

    for i in range(container.parameterCount()):
        p = container.parameter(i)

        if p.symbol.stage == System.Environment.CS_UNDEFINED:
            continue

        params[p.variableName] = ",".join(p.symbol.values)

    return params

def logd(message):
    '''
    Debugding method
    '''
    if DEBUG:
        print(message, file=sys.stderr)
        sys.stderr.flush()

def log(message):
    '''
    Helper method for outputting with flushing
    '''
    print(message, file=sys.stdout)
    sys.stdout.flush()

class InventoryResolver(object):
    def __init__(self, inventory):
        self._inventory = inventory
        pass

    '''
        Those should be internal methods only 
    '''
    def _overlaps(self, pstart, pend, cstart, cend):
        if cstart is None and cend is None: return True
        
        if cstart is None:
            cstart = Core.Time()
        
        if pend is not None:
            if pend > cstart:
                if cend is None or pstart < cend:
                    return True
        else:
            if cend is None or pstart < cend:
                return True
    
        return False

    def _getEnd(self, obj):
        try:
            return obj.end()
        except ValueError:
            return None

    def _codeMatch(self, obj, code):
        if not code: return True
        if fnmatch.fnmatch(str(obj.code()).upper(), code.strip().upper()): return True
        return False

    def _collect(self, objs, count, code, start, end):
        items = []

        for i in range(0, count):
            obj = objs(i)
    
            # Check code
            if not self._codeMatch(obj, code): continue 
    
            # Check time
            if not self._overlaps(obj.start(), self._getEnd(obj), start, end): continue
    
            items.append(obj)

        return items

    def _findStreams(self, location, code, start, end):
        items = self._collect(location.stream, location.streamCount(), code, start, end)
        if len(items) == 0:
            raise Exception("Location %s / %s does not have a stream named: %s in the time range %s / %s " % (location.code(), location.start(), code, start, end))
        return items

    def _findLocations(self, station, code, start, end):
        items = self._collect(station.sensorLocation, station.sensorLocationCount(), code, start, end)
        if len(items) == 0:
            raise Exception("Station %s / %s does not have a location named: %s in the time range %s / %s " % (station.code(), station.start(), code, start, end))
        return items

    def _findStations(self, network, code, start, end):
        items = self._collect(network.station, network.stationCount(), code, start, end)
        if len(items) == 0:
            raise Exception("Network %s / %s does not have a station named: %s in the time range %s / %s " % (network.code(), network.start(), code, start, end))
        return items

    def _findNetworks(self, code, start, end):
        items = self._collect(self._inventory.network, self._inventory.networkCount(), code, start, end)
        if len(items) == 0:
            raise Exception("Inventory does not have a network named: %s in the time range %s / %s " % (code, start, end))
        return items

    def _truncateDate(self, obj, currentDate):
        if currentDate < obj.start():
            return obj.start()
        end = self._getEnd(obj)
        if end and currentDate > end:
            return end
        return currentDate

    '''
        Public methods that should be used 
    '''
    def findStartDate(self, network, start, end):
        if start is None:
            return network.start()
        return self._truncateDate(network, start)

    def findEndDate(self, network, start, end):
        if end is None:
            try: return network.end()
            except ValueError: return None

        return self._truncateDate(network, end)

    def expandStream(self, stations, streams, start, end):
        items = []
    
        for strm in streams.split(','):
            (locationCode, streamCode) = ('.' + strm).split('.')[-2:]
            for station in stations:
                try:
                    for location in self._findLocations(station, locationCode, start, end):
                        if locationCode:
                            currentLocCode = location.code()
                        else:
                            currentLocCode = ""
                        try:
                            for stream in self._findStreams(location, streamCode, start, end):
                                try:
                                    items.index((currentLocCode, stream.code()))
                                except:
                                    items.append((currentLocCode, stream.code()))
                        except Exception as e:
                            pass
                except Exception as e:
                    pass
    
        return items

    def expandNetworkStation(self, ncode, scode, start, end):
        items = []

        for network in self._findNetworks(ncode, start, end):

            try:
                stations = self._findStations(network, scode, start, end)
            except Exception as  e:
                logd(str(e))
                continue

            # Append
            items.append((network, stations))
    
        if len(items) == 0:
            raise Exception("Cannot find suitable %s network with station code %s ranging from %s / %s" % (ncode, scode, start, end))
        return items

class RoutingDBUpdater(Client.Application):
    def __init__(self, argc, argv):
        Client.Application.__init__(self, argc, argv)
        self.setLoggingToStdErr(True)
        self.setMessagingEnabled(True)
        self.setDatabaseEnabled(True, True)
        self.setAutoApplyNotifierEnabled(False)
        self.setInterpretNotifierEnabled(False)
        self.setMessagingUsername("_sccfgupd_")
        ##self.setLoadConfigModuleEnabled(True)
        # Load all configuration modules
        ##self.setConfigModuleName("")
        self.setPrimaryMessagingGroup(Communication.Protocol.LISTENER_GROUP)

    def send(self, *args):
        '''
        A simple wrapper that sends a message and tries to resend it in case of
        an error.
        '''
        while not self.connection().send(*args):
            log("sending failed, retrying")
            time.sleep(1)

    def sendNotifiers(self, group):
        Nsize = DataModel.Notifier.Size()

        if Nsize > 0:
            logd("trying to apply %d changes..." % Nsize)
        else:
            logd("no changes to apply")
            return

        Nmsg = DataModel.Notifier.GetMessage(True)

        it = Nmsg.iter()
        msg = DataModel.NotifierMessage()

        maxmsg = 100
        sent = 0
        mcount = 0

        try:
            try:
                while it.get():
                    msg.attach(DataModel.Notifier.Cast(it.get()))
                    mcount += 1
                    if msg and mcount == maxmsg:
                        sent += mcount
                        logd("sending message (%5.1f %%)" % (sent / float(Nsize) * 100.0))
                        self.send(group, msg)
                        msg.clear()
                        mcount = 0
                        self.sync("_sccfgupd_")

                    it.next()
            except:
                pass
        finally:
            if msg.size():
                logd("sending message (%5.1f %%)" % 100.0)
                self.send(group, msg)
                msg.clear()
                self.sync("_sccfgupd_")

    def run(self):
        '''
        Reimplements the main loop of the application. This methods collects
        all bindings and updates the database. It searches for already existing
        objects and updates them or creates new objects. Objects that is didn't
        touched are removed. This tool is the only one that should writes the
        configuration into the database and thus manages the content.
        '''
        # Initialize the basic directories
        filebase = System.Environment.Instance().installDir()
        descdir = os.path.join(filebase, "etc", "descriptions")
        keydir = os.path.join(filebase, "etc", "key", self.name())

        # Load definitions of the configuration schema
        defs = System.SchemaDefinitions()
        if defs.load(descdir) == False:
            log("could not read descriptions")
            return False

        if defs.moduleCount() == 0:
            log("no modules defined, nothing to do")
            return False

        # Create a model from the schema and read its configuration including
        # all bindings.
        model = System.Model()
        model.create(defs)
        model.readConfig()

        mod = model.module("arclink")
        mod_access = model.module("arclink-access")

        existingRoutes = {}
        existingAccess  = {}

        routing = self.query().loadRouting()
        inventory = self.query().loadInventory()
        iResolver = InventoryResolver(inventory)

        DataModel.Notifier.Enable()
        DataModel.Notifier.SetCheckEnabled(False)

        if mod:
            log("Working on arclink bindings")
            for staid in mod.bindings.keys():
                binding = mod.getBinding(staid)
                if not binding:
                    log("no route to station %s %s" % (staid.networkCode, staid.stationCode))
                    continue
    
                params = {}
                for i in range(binding.sectionCount()):
                    params.update(collectParams(binding.section(i)))

                if 'routes' not in params:
                    log("no routes definition to station %s %s" % (staid.networkCode, staid.stationCode))
                    continue
    
                a_def_priority = 1
                s_def_priority = 1
    
                for r in params['routes'].split(','):
                    emptyLabel = True

                    ## Default is None
                    route_streams = params.get('routes.%s.streams' % r)
                    ## DEFAULT is True
                    route_netonly = params.get('routes.%s.disableStationCode' % r)
                    if route_netonly is None or route_netonly == "false":
                        route_netonly = False
                    else:
                        route_netonly = True

                    # Arclink
                    route_address = params.get('routes.%s.arclink.address' % r)
                    if route_address:
                        emptyLabel = False
                        route_start = params.get('routes.%s.arclink.start' % r)
                        route_end = params.get('routes.%s.arclink.end' % r)
                        route_priority = params.get('routes.%s.arclink.priority' % r)
                        
                        networkCode = staid.networkCode
                        stationCode = staid.stationCode
                        
                        if route_priority is None:
                            route_priority = a_def_priority
                        else:
                            route_priority = int(route_priority)
                        
                        if route_end:
                            route_end = Core.Time.FromString(route_end, "%Y-%m-%d %H:%M:%S")
                        
                        if route_start:
                            route_start = Core.Time.FromString(route_start, "%Y-%m-%d %H:%M:%S")
                        
                        if route_netonly:
                            stationCode = ""
                        
                        ## Resolve Inventory
                        try:
                            networkList = iResolver.expandNetworkStation(networkCode, stationCode, route_start, route_end)
                        except Exception as  e:
                            log("Arclink routing issue, cannot find network object for %s %s (label: %s)::\n\t %s" % (staid.networkCode, staid.stationCode, r, str(e)))
                            continue
                        
                        ## Generate routes for each network found
                        for (network, stations) in networkList:
                            
                            ## Resolve start date / end date of routing to be generated
                            rStart = iResolver.findStartDate(network, route_start, route_end)
                            rEnd   = iResolver.findEndDate(network, route_start, route_end)
                            
                            if not route_streams:
                                existingRoutes[('A', networkCode, stationCode, "", "", route_address, rStart.toString("%Y-%m-%d %H:%M:%S"))] = (rEnd, route_priority)
                                logd("Adding %s.%s.%s.%s" % (networkCode, stationCode, "", ""))
                                continue
                            
                            ## Add the route or routes for this net
                            for (locationCode, streamCode) in iResolver.expandStream(stations, route_streams, route_start, route_end):
                                existingRoutes[('A', networkCode, stationCode, locationCode, streamCode, route_address, rStart.toString("%Y-%m-%d %H:%M:%S"))] = (rEnd, route_priority)
                                logd("Adding %s.%s.%s.%s" % (networkCode, stationCode, locationCode, streamCode))
                        
                        a_def_priority += 1
                    else:
                        logd("binding for %s %s is missing routes.%s.arclink.address" % (staid.networkCode, staid.stationCode, r))
    
                    ## Seedlink
                    route_address = params.get('routes.%s.seedlink.address' % r)
                    if route_address:
                        emptyLabel = False
                        route_priority = params.get('routes.%s.seedlink.priority' % r)
                        
                        networkCode = staid.networkCode
                        stationCode = staid.stationCode
        
                        if route_priority is None:
                            route_priority = s_def_priority
        
                        else:
                            route_priority = int(route_priority)
    
                        if route_netonly:
                            stationCode = ""
    
                        ## Resolve Inventory
                        try:
                            networkList = iResolver.expandNetworkStation(networkCode, stationCode, Core.Time.GMT(), None)
                        except Exception as  e:
                            log("Seedlink routing issue, cannot find network object for %s %s (label: %s)::\n\t %s" % (staid.networkCode, staid.stationCode, r, str(e)))
                            continue
    
                        ## Generate routes for each network found
                        for (network, stations) in networkList:
                            
                            
                            if not route_streams:
                                existingRoutes[('S', networkCode, stationCode, "", "", route_address, None)] = (None, route_priority)
                                continue
                            
                            ## Add the route or routes for this net
                            for (locationCode, streamCode) in iResolver.expandStream(stations, route_streams, Core.Time.GMT(), None):
                                existingRoutes[('S', networkCode, stationCode, locationCode, streamCode, route_address, None)] = (None, route_priority)
    
                        
                        s_def_priority += 1
                    else:
                        logd("seedlink binding of station %s is missing routes.%s.seedlink.address" % (staid.networkCode + '_' + staid.stationCode, r))
                    
                    if emptyLabel:
                        log("routes label %s is empty or not found for station %s.%s" % (r,staid.networkCode, staid.stationCode))

        # Update access on basis of access module
        if mod_access:
            logd("Working on arclink-access bindings")
            for staid in mod_access.bindings.keys():
                binding = mod_access.getBinding(staid)
                if not binding: continue

                params = {}
                for i in range(binding.sectionCount()):
                    params.update(collectParams(binding.section(i)))

                access_users = params.get('access.users')
                access_start = params.get('access.start')
                access_end = params.get('access.end')
                access_netonly = params.get('access.disableStationCode')
                access_streams = params.get('access.streams')

                if access_netonly is None or access_netonly == "false":
                    access_netonly = False
                else:
                    access_netonly = True

                if not access_users: continue

                networkCode = staid.networkCode
                stationCode = staid.stationCode

                if access_start:
                    access_start = Core.Time.FromString(access_start, "%Y-%m-%d %H:%M:%S")

                if access_end:
                    access_end = Core.Time.FromString(access_end, "%Y-%m-%d %H:%M:%S")

                if access_netonly:
                    stationCode = ""
                
                ## Resolve Inventory
                try:
                    networkList = iResolver.expandNetworkStation(networkCode, stationCode, access_start, access_end)
                except Exception as  e:
                    #log("Access issue, cannot find network object for %s %s::\n\t %s" % (staid.networkCode, staid.stationCode, str(e)))
                    for user in access_users.split(','):
                        existingAccess[(networkCode, "", "", "", user, "1980-01-01 00:00:00")] = (None,)
                    continue

                ## Generate routes for each network found
                for (network, stations) in networkList:
                    
                    ## Resolve start date / end date of routing to be generated
                    aStart = iResolver.findStartDate(network, access_start, access_end)
                    aEnd   = iResolver.findEndDate(network, access_start, access_end)

                    if not access_streams:
                        for user in access_users.split(','):
                            existingAccess[(networkCode, stationCode, "", "", user, aStart.toString("%Y-%m-%d %H:%M:%S"))] = (aEnd,)
                        continue
                    
                    ## Add the route or routes for this net
                    for (locationCode, streamCode) in iResolver.expandStream(stations, access_streams, access_start, access_end):
                        for user in access_users.split(','):
                            existingAccess[(networkCode, stationCode, locationCode, streamCode, user, aStart.toString("%Y-%m-%d %H:%M:%S"))] = (aEnd,)


        for ((routeType, networkCode, stationCode, locationCode, streamCode, address, start), (end, priority)) in existingRoutes.items():
            if routeType != 'A' and routeType != 'S':
                logd("Invalid route type %s " % routeType)
                continue

            route = routing.route(DataModel.RouteIndex(networkCode, stationCode, locationCode, streamCode))
            if not route:
                route = DataModel.Route.Create()
                route.setNetworkCode(networkCode)
                route.setStationCode(stationCode)
                route.setLocationCode(locationCode)
                route.setStreamCode(streamCode)
                routing.add(route)
            
            if routeType == 'A':
                arclink = route.routeArclink(DataModel.RouteArclinkIndex(address, Core.Time.FromString(start, "%Y-%m-%d %H:%M:%S")))
                if not arclink:
                    arclink = DataModel.RouteArclink()
                    arclink.setAddress(address)
                    arclink.setStart(Core.Time.FromString(start, "%Y-%m-%d %H:%M:%S"))
                    arclink.setEnd(end)
                    arclink.setPriority(priority)
                    logd("inserting: %s %s.%s.%s.%s %s %s %s %s" % (routeType, networkCode, stationCode, locationCode, streamCode, address, start, end, priority))
                    route.add(arclink)
                else:
                    logd("checking for update: %s %s.%s.%s.%s %s %s %s %s" % (routeType, networkCode, stationCode, locationCode, streamCode, address, start, end, priority))
                    
                    update = False
                    
                    try:
                        cpriority = arclink.priority()
                        if cpriority != priority:
                            arclink.setPriority(priority)
                            update = True
                    except ValueError as e:
                        if priority:
                            arclink.setPriority(priority)
                            update = True
                    
                    try:
                        cend = arclink.end()
                        if (not end) or (end and cend != end):
                            arclink.setEnd(end)
                            update = True
                    except ValueError as e:
                        if end:
                            arclink.setEnd(end)
                            update = True
                    
                    if update:
                        arclink.update()

            elif routeType == 'S':
                seedlink = route.routeSeedlink(DataModel.RouteSeedlinkIndex(address))
                if not seedlink:
                    seedlink = DataModel.RouteSeedlink()
                    seedlink.setAddress(address)
                    seedlink.setPriority(priority)
                    logd("inserting: %s %s.%s.%s.%s %s %s %s %s" % (routeType, networkCode, stationCode, locationCode, streamCode, address, start, end, priority))
                    route.add(seedlink)
                else:
                    logd("checking for update: %s %s.%s.%s.%s %s %s %s %s" % (routeType, networkCode, stationCode, locationCode, streamCode, address, start, end, priority))
                    
                    update = False
                    
                    try:
                        cpriority = seedlink.priority()
                        if cpriority != priority:
                            seedlink.setPriority(priority)
                            update = True
                    except ValueError as e:
                        if priority:
                            seedlink.setPriority(priority)
                            update = True
                    
                    if update:
                        seedlink.update()
            
            else:
                log("route (%s.%s.%s.%s %s,%s) not inserted because is of an invalid type." % (networkCode, stationCode, locationCode, streamCode, address, priority))

        i = 0
        while i < routing.routeCount():
            route = routing.route(i)
            j = 0
            while j < route.routeArclinkCount():
                arclink = route.routeArclink(j)
                if ('A', route.networkCode(), route.stationCode(), route.locationCode(), route.streamCode(), arclink.address(), arclink.start().toString("%Y-%m-%d %H:%M:%S")) not in existingRoutes:
                    route.removeRouteArclink(j)
                    logd("removing %s.%s.%s.%s %s" % (route.networkCode(), route.stationCode(), route.locationCode(), route.streamCode(), arclink.address()))
                    continue

                j += 1

            k = 0
            while k < route.routeSeedlinkCount():
                seedlink = route.routeSeedlink(k)
                if ('S', route.networkCode(), route.stationCode(), route.locationCode(), route.streamCode(), seedlink.address(), None) not in existingRoutes:
                    route.removeRouteSeedlink(k)
                    logd("removing %s.%s.%s.%s %s" % (route.networkCode(), route.stationCode(), route.locationCode(), route.streamCode(), seedlink.address()))
                    continue

                k += 1

            if ( j + k ) == 0:
                routing.removeRoute(i)
                continue

            i += 1

        for ((networkCode, stationCode, locationCode, streamCode, user, start), (end,)) in existingAccess.items():
            access = routing.access(DataModel.AccessIndex(networkCode, stationCode, locationCode, streamCode, user, Core.Time.FromString(start, "%Y-%m-%d %H:%M:%S")))
            if not access:
                access = DataModel.Access()
                access.setNetworkCode(networkCode)
                access.setStationCode(stationCode)
                access.setLocationCode(locationCode)
                access.setStreamCode(streamCode)
                access.setUser(user)
                access.setStart(Core.Time.FromString(start, "%Y-%m-%d %H:%M:%S"))
                access.setEnd(end)
                routing.add(access)
            else:
                update = False
                try:
                    cend = access.end()
                    if (not end) or (end and cend != end):
                        access.setEnd(end)
                        update = True
                except ValueError as e:
                    if end:
                        access.setEnd(end)
                        update = True
                
                if update:
                    access.update()


        i = 0
        while i < routing.accessCount():
            access = routing.access(i)
            if (access.networkCode(), access.stationCode(), access.locationCode(), access.streamCode(), access.user(), access.start().toString("%Y-%m-%d %H:%M:%S")) not in existingAccess:
                routing.remove(access)
                continue

            i += 1

        self.sendNotifiers("ROUTING")
        return True

class TemplateModule(Kernel.Module):
    def __init__(self, env):
        Kernel.Module.__init__(self, env, env.moduleName(__file__))

        self.pkgroot = self.env.SEISCOMP_ROOT
        envi = System.Environment_Instance()
        
        cfg = Config.Config()

        # Defaults Global + App Cfg
        cfg.readConfig(envi.globalConfigFileName("global"))
        cfg.readConfig(envi.globalConfigFileName(self.name))

        # Config Global + App Cfg
        cfg.readConfig(envi.appConfigFileName("global"))
        cfg.readConfig(envi.appConfigFileName(self.name))

        # User Global + App Cfg
        cfg.readConfig(envi.configFileName("global"))
        cfg.readConfig(envi.configFileName(self.name))

        #System.Environment.Instance().initConfig(cfg, self.name)
        self.global_params = dict([(x, ",".join(cfg.getStrings(x))) for x in cfg.names()])
        self.global_params_ex = dict(filter(lambda s: s[1].find("$") != -1, [(x, ",".join(cfg.getStrings(x))) for x in cfg.names()]))
        self.template_dir = os.path.join(self.pkgroot, "share", "templates", self.name)
        self.alt_template_dir = "" #os.path.join(self.env.home
        self.config_dir = os.path.join(self.pkgroot, "var", "lib", self.name)
        self.run_dir = os.path.join(self.pkgroot, "var", "run", self.name)
        self.bindings_dir = os.path.join(self.pkgroot, "etc", "key")
        self.key_dir = os.path.join(self.bindings_dir, self.name)

    def _process_template(self, tpl_file, source=None, print_error=False):
        tpl_paths = []

        if source:
            tpl_paths.append(os.path.join(self.alt_template_dir, source))
            tpl_paths.append(os.path.join(self.template_dir, source))

        tpl_paths.append(self.alt_template_dir)
        tpl_paths.append(self.template_dir)

        params = self.global_params.copy()
        params_ex = self.global_params_ex.copy()

        params['pkgroot'] = self.pkgroot

        for (p,v) in params_ex.items():
            try:
                t2 = Kernel.Template(v)
                params[p] = t2.substitute(params)

            except (KeyError, ValueError):
                pass

        return self.env.processTemplate(tpl_file, tpl_paths, params, print_error)

    def param(self, name, print_warning=False):
        try:
            return self.global_params[name]

        except KeyError:
            pass

        if print_warning:
            print("warning: parameter '%s' is not defined" % (name,), file=sys.stderr)

        raise KeyError

    def setParam(self, name, value):
        self._set(name, value)

    def _get(self, name):
        try: return self.param(name)
        except KeyError: return ""

    def _set(self, name, value):
        self.global_params[name] = value

class Module(TemplateModule):
    def __init__(self, env):
        TemplateModule.__init__(self, env)

    def _run(self):
        if self.env.syslog:
            daemon_opt = '-D '
        else:
            daemon_opt = ''

        daemon_opt += "-v -f " + os.path.join(self.config_dir, "arclink.ini")

        return self.env.start(self.name, self.env.binaryFile(self.name), daemon_opt,\
                              True)

    def _set_default(self, name, value):
        try: self.param(name)
        except: self._set(name, value)

    def updateConfig(self):
        # Set default values
        try: self._set_default("organization", self.env.getString("organization"))
        except: pass

        if self.env.syslog:
            syslog_opt = ' -s'
        else:
            syslog_opt = ''

        self._set('arclink._syslog_opt', syslog_opt)

        self._set_default("request_dir", "@ROOTDIR@/var/lib/arclink/requests")
        self._set_default("datacenterID", "TEST")
        self._set_default("contact_email", "")
        self._set_default("connections", 500)
        self._set_default("connections_per_ip", 20)
        self._set_default("request_queue", 500)
        self._set_default("request_queue_per_user", 10)
        self._set_default("request_size", 1000)
        self._set_default("handlers_soft", 4)
        self._set_default("handlers_hard", 10)
        self._set_default("handler_cmd", "@ROOTDIR@/share/plugins/arclink/reqhandler" + syslog_opt)
        self._set_default("handler_timeout", 10)
        self._set_default("handler_start_retry", 60)
        self._set_default("handler_shutdown_wait", 10)
        self._set_default("port", 18001)
        self._set_default("lockfile", "@ROOTDIR@/var/run/arclink.pid")
        self._set_default("statefile", "@ROOTDIR@/var/lib/arclink/arclink.state")
        self._set_default("admin_password", "")
        self._set_default("handlers_waveform", 2)
        self._set_default("handlers_response", 2)
        self._set_default("handlers_inventory", 2)
        self._set_default("handlers_routing", 2)
        self._set_default("handlers_qc", 2)
        self._set_default("handlers_greensfunc", 1)
        self._set_default("swapout_time", 600)
        self._set_default("purge_time", 864000)
        self._set_default("encryption", "false")
        self._set_default("password_file", "@ROOTDIR@/var/lib/arclink/password.txt")


        ## Expand the @Variables@
        e = System.Environment_Instance()
        self.setParam("password_file", e.absolutePath(self.param("password_file")))
        self.setParam("lockfile", e.absolutePath(self.param("lockfile")))
        self.setParam("statefile", e.absolutePath(self.param("statefile")))
        self.setParam("request_dir", e.absolutePath(self.param("request_dir")))
        self.setParam("handler_cmd", e.absolutePath(self.param("handler_cmd")))
        
        try: os.makedirs(self.config_dir)
        except: pass

        try: os.makedirs(self.run_dir)
        except: pass

        fd = open(os.path.join(self.config_dir, "arclink.ini"), "w")
        fd.write(self._process_template("arclink.tpl", None, False))
        fd.close()

        messaging = True
        messagingPort = 4803

        try: messaging = self.env.getBool("messaging.enable")
        except: pass
        try: messagingPort = self.env.getInt("messaging.port")
        except: pass

        # If messaging is disabled in kernel.cfg, do not do anything
        if not messaging:
          log("- messaging disabled, nothing to do")
          return 0

        # Synchronize database configuration
        params = [self.name, '--console', '1', '-H', 'localhost:%d' % messagingPort]
        # Create the database update app and run it
        # This app implements a Client.Application and connects
        # to localhost regardless of connections specified in global.cfg to
        # prevent updating a remote installation by accident.
        app = RoutingDBUpdater(len(params), params)
        return app()
