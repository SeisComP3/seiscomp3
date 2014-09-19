import os, string, time, re, glob, shutil, sys, imp, resource
import seiscomp3.Kernel, seiscomp3.Config
try:
    import seiscomp3.System
    hasSystem = True
except:
    hasSystem = False

try:
    import seiscomp3.DataModel
    import seiscomp3.IO
    dbAvailable = True
except:
    dbAvailable = False


'''
NOTE:
The plugin to be used for a station of configured with:
plugin = [type]
All plugin specific parameters are stored in plugin.[type].*.

All parameters from seedlink.cfg are not prefixed with "seedlink.".
Local parameters that are created from seedlink.cfg parameters are
prefixed with "seedlink.".

NOTE2: Support a database connection to get station descriptions.
'''

def _loadDatabase(dbUrl):
    """
    Load inventory from a database, but only down to the station level.
    """
    m = re.match("(?P<dbDriverName>^.*):\/\/(?P<dbAddress>.+?:.+?@.+?\/.+$)", dbUrl)
    if not m:
        raise Exception("error in parsing SC3 DB URL")
    db = m.groupdict()
    try:
        registry = seiscomp3.Client.PluginRegistry.Instance()
        registry.addPluginName("db" + db["dbDriverName"])
        registry.loadPlugins()
    except Exception, e:
        raise(e) ### "Cannot load database driver: %s" % e)
    dbDriver = seiscomp3.IO.DatabaseInterface.Create(db["dbDriverName"])
    if dbDriver is None:
        raise Exception("Cannot find database driver " + db["dbDriverName"])
    if not dbDriver.connect(db["dbAddress"]):
        raise Exception("Cannot connect to database at " + db["dbAddress"])
    dbQuery = seiscomp3.DataModel.DatabaseQuery(dbDriver)
    if dbQuery is None:
        raise Exception("Cannot get DB query object")
    print >> sys.stderr, " Loading inventory from database ... ",
    inventory = seiscomp3.DataModel.Inventory()
    dbQuery.loadNetworks(inventory)
    for ni in xrange(inventory.networkCount()):
        dbQuery.loadStations(inventory.network(ni))
    print >> sys.stderr, "Done."
    return inventory


def _loadStationDescriptions(inv):
    """From an inventory, prepare a dictionary of station code descriptions.

    In theory, we should only use stations with current time windows.

    """
    d = dict()

    for ni in xrange(inv.networkCount()):
        n = inv.network(ni)
        net = n.code()
        if not d.has_key(net):
            d[net] = {}

            for si in xrange(n.stationCount()):
                s = n.station(si)
                sta = s.code()
                d[net][sta] = s.description()

                try:
                    end = s.end()
                except:  # ValueException ???
                    end = None
                #print "Found in inventory:", net, sta, end, s.description()
    return d

class TemplateModule(seiscomp3.Kernel.Module):
    def __init__(self, env):
        seiscomp3.Kernel.Module.__init__(self, env, env.moduleName(__file__))

        self.pkgroot = self.env.SEISCOMP_ROOT

        cfg = seiscomp3.Config.Config()

        # Defaults Global + App Cfg
        cfg.readConfig(os.path.join(self.pkgroot, "etc", "defaults", "global.cfg"))
        cfg.readConfig(os.path.join(self.pkgroot, "etc", "defaults", self.name + ".cfg"))

        # Config Global + App Cfg
        cfg.readConfig(os.path.join(self.pkgroot, "etc", "global.cfg"))
        cfg.readConfig(os.path.join(self.pkgroot, "etc", self.name + ".cfg"))

        # User Global + App Cfg
        cfg.readConfig(os.path.join(os.environ['HOME'], ".seiscomp3", "global.cfg"))
        cfg.readConfig(os.path.join(os.environ['HOME'], ".seiscomp3", self.name + ".cfg"))

        self.global_params = dict([(x, ",".join(cfg.getStrings(x))) for x in cfg.names()])
        self.station_params = dict()
        self.plugin_dir = os.path.join(self.pkgroot, "share", "plugins", "seedlink")
        self.template_dir = os.path.join(self.pkgroot, "share", "templates", "seedlink")
        self.alt_template_dir = "" #os.path.join(self.env.home
        self.config_dir = os.path.join(self.pkgroot, "var", "lib", self.name)

        self.database_str = ""
        if self.global_params.has_key("inventory_connection"):
            #WRONG self.database_str = cfg.getStrings("seedlink.readConnection")
            self.database_str = self.global_params["inventory_connection"]
        #self.database_str = cfg.getStrings("seedlink.database.type")+cfg.getStrings("seedlink.database.parameters")

        self.seedlink_station_descr = dict()
        self.rc_dir = os.path.join(self.pkgroot, "var", "lib", "rc")
        self.run_dir = os.path.join(self.pkgroot, "var", "run", self.name)
        self.bindings_dir = os.path.join(self.pkgroot, "etc", "key")
        self.key_dir = os.path.join(self.bindings_dir, self.name)
        self.net = None
        self.sta = None

    def _read_station_config(self, cfg_file):
        cfg = seiscomp3.Config.Config()
        cfg.readConfig(os.path.join(self.key_dir, cfg_file))
        self.station_params = dict([(x, ",".join(cfg.getStrings(x))) for x in cfg.names()])
        #self.station_params_ex = dict(filter(lambda s: s[1].find("$") != -1, [(x, ",".join(cfg.getStrings(x))) for x in cfg.names()]))

    def _process_template(self, tpl_file, source=None, station_scope=True, print_error=True):
        tpl_paths = []

        if source:
            tpl_paths.append(os.path.join(self.alt_template_dir, source))
            tpl_paths.append(os.path.join(self.template_dir, source))

        tpl_paths.append(self.alt_template_dir)
        tpl_paths.append(self.template_dir)

        params = self.global_params.copy()
        #params_ex = self.global_params_ex.copy()

        if station_scope:
            params.update(self.station_params)
            #params_ex.update(self.station_params_ex)

        params['pkgroot'] = self.pkgroot

        #for (p,v) in params_ex.iteritems():
        #    try:
        #        t2 = seiscomp3.Kernel.Template(v)
        #        params[p] = t2.substitute(params)
        #
        #    except (KeyError, ValueError):
        #        pass

        return self.env.processTemplate(tpl_file, tpl_paths, params, print_error)

    def param(self, name, station_scope=True, print_warning=False):
        if station_scope:
            try:
                return self.station_params[name]

            except KeyError:
                pass
        else:
            try:
                return self.global_params[name]

            except KeyError:
                pass

        if print_warning:
            if station_scope:
                print "warning: parameter '%s' is not defined for station %s %s" % (name, self.net, self.sta)
            else:
                print "warning: parameter '%s' is not defined at global scope" % (name,)

        raise KeyError

    def setParam(self, name, value, station_scope=True):
        self._set(name, value, station_scope)

    def _get(self, name, station_scope=True):
        try: return self.param(name, station_scope)
        except KeyError: return ""

    def _set(self, name, value, station_scope=True):
        if station_scope:
            self.station_params[name] = value

        else:
            self.global_params[name] = value

class Module(TemplateModule):
    def __init__(self, env):
        TemplateModule.__init__(self, env)
        # Set kill timeout to 5 minutes
        self.killTimeout = 300

    def _run(self):
        if self.env.syslog:
            daemon_opt = '-D '
        else:
            daemon_opt = ''

        daemon_opt += "-v -f " + os.path.join(self.config_dir, "seedlink.ini")

        try:
            lim = resource.getrlimit(resource.RLIMIT_NOFILE)
            resource.setrlimit(resource.RLIMIT_NOFILE, (lim[1], lim[1]))

            lim = resource.getrlimit(resource.RLIMIT_NOFILE)
            print >> sys.stderr, " maximum number of open files set to", lim[0]

        except Exception, e:
            print >> sys.stderr, " failed to raise the maximum number open files:", str(e)

        if self.global_params.has_key("sequence_file_cleanup"):
            try:
                max_minutes = int(self.global_params["sequence_file_cleanup"])
                if max_minutes > 0:
                    files = glob.glob(os.path.join(self.run_dir, "*.seq"))
                    for f in files:
                        if (time.time()-os.path.getmtime(f))/60 >= max_minutes:
                            print >> sys.stderr, " removing sequence file %s" % f
                            os.remove(f)
                else:
                    print >>sys.stderr, " sequence_file_cleanup disabled"

            except ValueError:
                print >>sys.stderr, " sequence_file_cleanup parameter is not a number: '%s'" % str(self.global_params["sequence_file_cleanup"])
                return 1

        return self.env.start(self.name, self.env.binaryFile(self.name), daemon_opt,\
                              not self.env.syslog)

    def _getPluginHandler(self, source_type):
        try:
            return self.plugins[source_type]
        except KeyError:
            path = os.path.join(self.template_dir, source_type, "setup.py")
            try: f = open(path, 'r')
            except: return None

            modname = '__seiscomp_seedlink_plugins_' + source_type
            if sys.modules.has_key(modname):
                mod = sys.modules[modname]
            else:
                # create a module
                mod = imp.new_module(modname)
                mod.__file__ = path

                # store it in sys.modules
                sys.modules[modname] = mod

            # our namespace is the module dictionary
            namespace = mod.__dict__

            # test whether this has been done already
            if not hasattr(mod, 'SeedlinkPluginHandler'):
                code = f.read()
                # compile and exec dynamic code in the module
                exec compile(code, '', 'exec') in namespace

            mod = namespace.get('SeedlinkPluginHandler')
            handler = mod()
            self.plugins[source_type] = handler
            return handler

    def _generateStationForIni(self):
        ini =  'station %s  description = "%s"\n' % \
               (self._get('seedlink.station.id'), self._get('seedlink.station.description'))
        ini += '             name = "%s"\n' % self._get('seedlink.station.code')
        ini += '             network = "%s"\n' % self._get('seedlink.station.network')
        if self._get('seedlink.station.sproc'):
            ini += '             proc = "%s"\n' % self._get('seedlink.station.sproc')
        if self._get('seedlink.station.access'):
            ini += '             access = "%s"\n' % self._get('seedlink.station.access').replace(',',' ')
        if self._get('seedlink.station.blanks'):
            ini += '             blanks = "%s"\n' % self._get('seedlink.station.blanks')
        if self._get('seedlink.station.encoding'):
            ini += '             encoding = "%s"\n' % self._get('seedlink.station.encoding')
        if self._get('seedlink.station.buffers'):
            ini += '             buffers = "%s"\n' % self._get('seedlink.station.buffers')
        if self._get('seedlink.station.segments'):
            ini += '             segments = "%s"\n' % self._get('seedlink.station.segments')
        if self._get('seedlink.station.segsize'):
            ini += '             segsize = "%s"\n' % self._get('seedlink.station.segsize')
        if self._get('seedlink.station.backfill_buffer'):
            ini += '             backfill_buffer = "%s"\n' % self._get('seedlink.station.backfill_buffer')
        ini += '\n'
        return ini

    def __process_station(self, profile):
        try:
            station_dict = self.seedlink_station[self.sta]
            station_id = self.sta + str(len(station_dict))

        except KeyError:
            station_dict = {}
            station_id = self.sta
            self.seedlink_station[self.sta] = station_dict

        if profile:
            self.station_config_file = "profile_%s" % (profile,)
        else:
            self.station_config_file = "station_%s_%s" % (self.net, self.sta)

        self._read_station_config(self.station_config_file)

        # Generate plugin independent parameters
        self._set('seedlink.station.id', station_id)
        self._set('seedlink.station.code', self.sta)
        self._set('seedlink.station.network', self.net)
        self._set('seedlink.station.access', self._get('access'))
        self._set('seedlink.station.sproc', self._get('proc'))
        self._set('seedlink.station.blanks', self._get('blanks'))
        self._set('seedlink.station.encoding', self._get('encoding'))
        self._set('seedlink.station.buffers', self._get('buffers'))
        self._set('seedlink.station.segments', self._get('segments'))
        self._set('seedlink.station.segsize', self._get('segsize'))
        self._set('seedlink.station.backfill_buffer', self._get('backfill_buffer'))

        # Supply station description:
        # 1. try getting station description from a database
        # 2. read station description from seiscomp3/var/lib/rc/station_NET_STA
        # 3. if not set, use the station code

        description = ""

        if len(self.seedlink_station_descr) > 0:
            try:
                description = self.seedlink_station_descr[self.net][self.sta]
            except KeyError:
                pass

        if len(description) == 0:
            try:
                rc = seiscomp3.Config.Config()
                rc.readConfig(os.path.join(self.rc_dir, "station_%s_%s" % (self.net, self.sta)))
                description = rc.getString("description")
            except Exception, e:
                # Maybe the rc file doesn't exist, maybe there's no readable description.
                pass

        if len(description) == 0:
            description = self.sta

        self._set('seedlink.station.description', description)

        self.station_count += 1

        if self._last_net != self.net:
            print "+ network %s" % self.net
            self._last_net = self.net

        print "  + station %s %s" % (self.sta, description)

        # If real-time simulation is activated do not parse the sources
        # and force the usage of the mseedfifo_plugin
        if self.msrtsimul:
            self._set('seedlink.station.sproc', '')
            station_dict[(self.net, self.sta)] = self._generateStationForIni()
            return

        station_sproc = set()

        for source_type in self._get('sources').split(','):
            if not source_type: continue

            source_alias = source_type
            toks = source_type.split(':')
            if len(toks) > 2:
                print "Error: invalid source identifier '%s', expected '[alias:]type'"
                continue
            elif len(toks) == 2:
                source_alias = toks[0]
                source_type = toks[1]

            # Plugins are outsourced to external handlers
            # that can be added with new plugins.
            # This requires a handler file:
            # share/templates/seedlink/$type/setup.py
            pluginHandler = self._getPluginHandler(source_type)
            if pluginHandler is None:
                print "Error: no handler for plugin %s defined" % source_type
                continue

            stat = source_type
            if source_alias != source_type:
                stat += " as " + source_alias

            print "    + source %s" % stat

            # Backup original binding parameters
            station_params = self.station_params.copy()
            #station_params_ex = self.station_params_ex.copy()

            # Modify parameter set. Remove alias definition with type string
            if source_type != source_alias:
                tmp_dict = {}
                for x in self.station_params.keys():
                    if x.startswith('sources.%s.' % source_type): continue
                    if x.startswith('sources.%s.' % source_alias):
                        toks = x.split('.')
                        toks[1] = source_type
                        tmp_dict[".".join(toks)] = self.station_params[x]
                    else:
                        tmp_dict[x] = self.station_params[x]
                self.station_params = tmp_dict

                #tmp_dict = {}
                #for x in self.station_params_ex.keys():
                #    if x.startswith('sources.%s.' % source_type): continue
                #    if x.startswith('sources.%s.' % source_alias):
                #        toks = x.split('.')
                #        toks[1] = source_type
                #        tmp_dict[".".join(toks)] = self.station_params_ex[x]
                #    else:
                #        tmp_dict[x] = self.station_params_ex[x]
                #self.station_params_ex = tmp_dict

            # Create source entry that ends up in seedlink.ini as plugin
            try:
                source_dict = self.seedlink_source[source_type]

            except KeyError:
                source_dict = {}
                self.seedlink_source[source_type] = source_dict

            source_key = pluginHandler.push(self)
            if source_key is None:
                source_key = source_type
            else:
                source_key = (source_type, source_key)

            if not source_dict.has_key(source_key):
                source_id = source_type + str(len(source_dict))

            else:
                (source_type, source_id) = source_dict[source_key][:2]

            # Update internal parameters usable by a template
            self._set('seedlink.source.type', source_type)
            self._set('seedlink.source.id', source_id)
            source_dict[source_key] = (source_type, source_id, self.global_params.copy(), self.station_params.copy())

            # Create procs for this type for streams.xml
            sproc_names = self._get('sources.%s.proc' % (source_type)) or self._get('proc')
            if sproc_names:
                sproc_names = [x.strip() for x in sproc_names.split(",")]
                for sproc_name in sproc_names:
                    self.sproc_used = True
                    sproc = self._process_template("streams_%s.tpl" % sproc_name, source_type, True, False)
                    if sproc:
                        station_sproc.add(sproc_name)
                        self.sproc[sproc_name] = sproc
                    else:
                        print "WARNING: cannot find streams_%s.tpl" % sproc_name

            # Read plugins.ini template for this source and store content
            # under the provided key for this binding
            plugin_ini = self._process_template("plugins.ini.tpl", source_type, True, False)
            if plugin_ini:
                self.plugins_ini[source_key] = plugin_ini

            templates = self._get('sources.%s.templates' % (source_type))
            if templates:
                for t in templates.split(','):
                    self.templates.add((t, source_type, 0))

            # Allow plugin handler to override station id
            station_params['seedlink.station.id'] = self.station_params['seedlink.station.id']

            # Set original parameters
            self.station_params = station_params

        if len(station_sproc) > 1:
            data = '  <proc name="%s">\n' % (",".join(station_sproc),)
            for name in station_sproc:
                data += '    <using proc="%s"/>\n' % (name,)

            data += '  </proc>\n'

            self._set('seedlink.station.sproc', ",".join(station_sproc))
            self.combined_sproc[",".join(station_sproc)] = data

        elif station_sproc:
            self._set('seedlink.station.sproc', list(station_sproc)[0])

        # Create station section for seedlink.ini
        station_dict[(self.net, self.sta)] = self._generateStationForIni()

    def __load_stations(self):
        self.seedlink_source = {}
        self.seedlink_station = {}
        self.plugins_ini = {}
        self.sproc = {}
        self.combined_sproc = {}
        self.plugins = {}
        self.sproc_used = False
        self.station_count = 0

        if self.env.syslog:
            self._set('seedlink._daemon_opt', ' -D', False)
        else:
            self._set('seedlink._daemon_opt', '', False)

        self._set('seedlink.plugin_dir', self.plugin_dir, False)
        self._set('seedlink.config_dir', self.config_dir, False)
        self._set('seedlink.run_dir', self.run_dir, False)
        self._set('seedlink.filters', os.path.join(self.config_dir, "filters.fir"), False)
        self._set('seedlink.streams', os.path.join(self.config_dir, "streams.xml"), False)

        self.templates = set()
        self.templates.add(("backup_seqfiles", None, 0755))

        rx_binding = re.compile(r'(?P<module>[A-Za-z0-9_\.-]+)(:(?P<profile>[A-Za-z0-9_-]+))?$')

        files = glob.glob(os.path.join(self.bindings_dir, "station_*"))
        files.sort()
        self._last_net = ""

        for f in files:
            try:
                (path, net, sta) = f.split('_')[-3:]
                if not path.endswith("station"):
                    print "invalid path", f

            except ValueError:
                print "invalid path", f
                continue

            self.net = net
            self.sta = sta

            fd = open(f)
            line = fd.readline()
            while line:
                line = line.strip()
                if not line or line[0] == '#':
                    line = fd.readline()
                    continue

                m = rx_binding.match(line)
                if not m:
                    print "invalid binding in %s: %s" % (f, line)
                    line = fd.readline()
                    continue

                if m.group('module') != self.name:
                    line = fd.readline()
                    continue

                profile = m.group('profile')
                self.__process_station(profile)
                break

            fd.close()

    def _set_default(self, name, value, station_scope = True):
        try: self.param(name, station_scope)
        except: self._set(name, value, station_scope)

    def supportsAliases(self):
        return True

    def updateConfig(self):
        # Set default values
        try: self._set_default("organization", self.env.getString("organization"), False)
        except: pass

        self._set_default("lockfile", os.path.join("@ROOTDIR@", self.env.lockFile(self.name)), False)
        self._set_default("filebase", os.path.join("@ROOTDIR@", "var", "lib", self.name, "buffer"), False)
        self._set_default("port", "18000", False)
        self._set_default("encoding", "steim2", False)
        self._set_default("trusted", "127.0.0.0/8", False)
        self._set_default("stream_check", "true", False)
        self._set_default("window_extraction", "true", False)
        self._set_default("window_extraction_trusted", "true", False)

        self._set_default("buffers", "100", False)
        self._set_default("segments", "50", False)
        self._set_default("segsize", "1000", False)

        self._set_default("gap_check_pattern", "", False)
        self._set_default("gap_treshold", "", False)

        self._set_default("info", "streams", False)
        self._set_default("info_trusted", "all", False)
        self._set_default("request_log", "true", False)
        self._set_default("proc_gap_warn", "10", False)
        self._set_default("proc_gap_flush", "100000", False)
        self._set_default("proc_gap_reset", "1000000", False)
        self._set_default("backfill_buffer", "0", False)
        self._set_default("seq_gap_limit", "100000", False)
        self._set_default("connections", "500", False)
        self._set_default("connections_per_ip", "20", False)
        self._set_default("bytespersec", "0", False)

        ## Expand the @Variables@
        if hasSystem:
            e = seiscomp3.System.Environment.Instance()
            self.setParam("filebase", e.absolutePath(self.param("filebase", False)), False)
            self.setParam("lockfile", e.absolutePath(self.param("lockfile", False)), False)
        else:
            self.setParam("filebase", self.param("filebase", False), False)
            self.setParam("lockfile", self.param("lockfile", False), False)

        if self._get("msrtsimul", False).lower() == "true":
          self.msrtsimul = True
        else:
          self.msrtsimul = False

        # Load custom stream processor definitions
        custom_procs = self._process_template("streams_custom.tpl", None, True, False)
        if custom_procs: self.sproc[""] = sproc

        # Load descriptions from inventory:
        if self.database_str:
            if dbAvailable == True:
                print >>sys.stderr, " Loading station descriptions from %s" % self.database_str
                inv = _loadDatabase(self.database_str)
                self.seedlink_station_descr = _loadStationDescriptions(inv)
            else:
                print >>sys.stderr, " Database configured but trunk is not installed"
                self.seedlink_station_descr = dict()

        self.__load_stations()

        if self.msrtsimul:
            self.seedlink_source['mseedfifo'] = {1:('mseedfifo',1,self.global_params.copy(),{})}

        try: os.makedirs(self.config_dir)
        except: pass

        try: os.makedirs(self.run_dir)
        except: pass

        for p in self.plugins.itervalues():
            p.flush(self)

        if self._get("stream_check", False).lower() == "true":
            self._set("stream_check", "enabled", False)
        else:
            self._set("stream_check", "disabled", False)

        if self._get("window_extraction", False).lower() == "true":
            self._set("window_extraction", "enabled", False)
        else:
            self._set("window_extraction", "disabled", False)

        if self._get("window_extraction_trusted", False).lower() == "true":
            self._set("window_extraction_trusted", "enabled", False)
        else:
            self._set("window_extraction_trusted", "disabled", False)

        if self._get("request_log", False).lower() == "true":
            self._set("request_log", "enabled", False)
        else:
            self._set("request_log", "disabled", False)

        self._set("name", self.name, False)
        fd = open(os.path.join(self.config_dir, "seedlink.ini"), "w")
        fd.write(self._process_template("seedlink_head.tpl", None, False))

        if self.sproc_used:
            fd.write(self._process_template("seedlink_sproc.tpl", None, False))

        for i in self.seedlink_source.itervalues():
            for (source_type, source_id, self.global_params, self.station_params) in i.itervalues():
                source = self._process_template("seedlink_plugin.tpl", source_type)
                if source:
                    fd.write(source)

        fd.write(self._process_template("seedlink_station_head.tpl", None, False))

        for i in self.seedlink_station.itervalues():
            for j in i.itervalues():
                fd.write(j)

        fd.close()

        if self.plugins_ini:
            fd = open(os.path.join(self.config_dir, "plugins.ini"), "w")
            for i in self.plugins_ini.itervalues():
                fd.write(i)

            fd.close()
        else:
            # If no plugins.ini is not used remove it from previous runs
            try: os.remove(os.path.join(self.config_dir, "plugins.ini"))
            except: pass

        if self.sproc_used:
            fd = open(self._get('seedlink.streams', False), "w")
            fd.write('<streams>\n')

            for i in self.sproc.itervalues():
                fd.write(i)

            for i in self.combined_sproc.itervalues():
                fd.write(i)

            fd.write('</streams>\n')
            fd.close()

            fd = open(self._get('seedlink.filters', False), "w")
            fd.write(self._process_template("filters.fir.tpl", None, False))
            fd.close()

        # If no stream procs are used, remove the generated files of a
        # previous run
        else:
            try: os.remove(self._get('seedlink.streams', False))
            except: pass
            try: os.remove(self._get('seedlink.filters', False))
            except: pass

        for (f, s, perm) in self.templates:
            fd = open(os.path.join(self.config_dir, f), "w")
            fd.write(self._process_template(f + '.tpl', s, False))
            fd.close()
            if perm:
                os.chmod(os.path.join(self.config_dir, f), perm)

        return 0


    def printCrontab(self):
        print "55 23 * * * %s >/dev/null 2>&1" % (os.path.join(self.config_dir, "backup_seqfiles"),)

