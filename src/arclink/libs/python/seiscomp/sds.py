#############################################################################
# sds.py                                                                    #
#                                                                           #
# ArcLink archive structure handler                                         #
#                                                                           #
# (c) 2008 Doreen Pahlke, GFZ Potsdam                                       #
#                                                                           #
# This program is free software; you can redistribute it and/or modify it   #
# under the terms of the GNU General Public License as published by the     #
# Free Software Foundation; either version 2, or (at your option) any later #
# version. For more information, see http://www.gnu.org/                    #
#                                                                           #
#############################################################################

import os, time, datetime
from seiscomp import logs
from optparse import OptionParser
from seiscomp3 import DataModel, Core, IO

try:
    import sqlite3
    have_sqlite = True

except ImportError:
    have_sqlite = False

class TemporaryUnavailabilityException(Exception):
    pass

class SDS(object):
    """
    This class implements the access to SEED data using the SeisComP Directory Structure.
    
    """
    
    def __init__(self, nrtdir, archdir, isodir, filedb=None):
        """
        Constructor of class SDS.
        
        @instance vars: nrtdir,  a string specifying the path to the near real time data
                        archdir, a string storing the path to the SDS archive
                        isodir,  a string defining the path to the ISO image archive
        """        
        self.nrtdir = nrtdir
        self.archdir = archdir
        self.isodir = isodir

        if filedb and have_sqlite:
            self.__cursor = sqlite3.connect(filedb).cursor()

        else:
            self.__cursor = False
        
    def __db_path_exists(self, path):
        self.__cursor.execute("select file from fileindex where dir=? and file=?", (os.path.dirname(path), os.path.basename(path)))
        if self.__cursor.fetchone():
            return True

        return False

    def __db_listdir(self, dir):
        self.__cursor.execute("select file from fileindex where dir=?", (dir,))
        return [ str([0]) for x in self.__cursor.fetchall() ]
        
    def __get_sds_name(self, timeobj, net, sta, cha, loc):
        """
        Constructs a SDS conform file name from the given parameters.
        
        @arguments: timeobj, a datetime object
                    net,     a string defining the network code
                    sta,     a string defining the station code
                    cha,     a string defining the channel code
                    loc,     a string defining the location ID
        @return: a string, giving the path to the SeisComP data structure;
                 the path doesn't need to exist
        """
        postfix = time.strftime("%Y.%j", timeobj.timetuple())
        name = "%d/%s/%s/%s.D/%s.%s.%s.%s.D.%s" % (timeobj.year, net, sta, cha, net, sta, loc, cha, postfix)

        return name


    def __get_iso_name(self, year, net, sta):
        """
        Constructs a file name corresponding to the archive name convention.
        
        @arguments: year, an integer representing the year
                    net,  a string defining the network code
                    sta,  a string defining the station code
        @return: a string storing the path to the ISO archive file
        """
        name = "%d/%s/%s.%s.%d.iso" % (year, net, sta, net, year)

        return name


    def __is_sds(self, time1, time2, net, sta, cha, loc, dir, f_path_exists, f_listdir):
        """
        Checks whether the given request elements can be found in the current archive SDS
        
        @arguments: time1, a datetime object; request start time
                    time2, a datetime object; request end time
                    net,   a string defining the network code
                    sta,   a string defining the station code
                    cha,   a string defining the channel code
                    loc,   a string defining the location ID
                    dir,   a string defining the corresponding archive root directory
        @return: True / False
        """
        paths = []
        for t in [time1, time2]:
            sdsfile = os.path.join(dir, self.__get_sds_name(t,net,sta,cha,loc))
            if f_path_exists(sdsfile):
                return True
            
            if time1.date() == time2.date():
                return False

            paths.append(sdsfile)

        sdoy = paths[0].split(".")[-1]
        edoy = paths[1].split(".")[-1]
        spath = os.path.dirname(paths[0])
        epath = os.path.dirname(paths[1])

        if f_path_exists(spath):
            for doy in [fname.split(".")[-1] for fname in f_listdir(spath)]:
                if doy >= sdoy and (spath != epath or doy <= edoy):
                    return True
        
        if f_path_exists(epath):
            for doy in [fname.split(".")[-1] for fname in f_listdir(epath)]:
                if doy <= edoy and (spath != epath or doy >= sdoy):
                    return True

        for i in range(1, time2.year-time1.year):
            t = time1.replace(year=time1.year+i)
            sdsfile = os.path.join(dir, self.__get_sds_name(t,net,sta,cha,loc))
            if f_path_exists(os.path.dirname(sdsfile)) and f_listdir(os.path.dirname(sdsfile)):
                return True
            
        return False
        

    def __is_iso(self, time1, time2, net, sta, f_path_exists):
        """
        Checks whether the given request elements can be found in the ISO9660 archive
        
        @arguments: time1, a datetime object specifying request start
                    time2, a datetime object specifying request end
                    net,   a string defining the network code
                    sta,   a string defining the station code
         @return: True / False
        """
        path = os.path.join(self.isodir,self.__get_iso_name(time1.year,net,sta))
        if f_path_exists(path):
            return True
        
        path = os.path.join(self.isodir,self.__get_iso_name(time2.year,net,sta))
        if f_path_exists(path):
            return True

        for i in range(1, time2.year-time1.year):
            path = os.path.join(self.isodir,self.__get_iso_name(time1.year+i,net,sta))
            if f_path_exists(path):
                return True

        return False


    def exists(self, time1, time2, net, sta, cha, loc):
        return self.__is_iso(time1, time2, net, sta, os.path.exists) or \
            self.__is_sds(time1, time2, net, sta, cha, loc, self.archdir, os.path.exists, os.listdir) or \
            self.__is_sds(time1, time2, net, sta, cha, loc, self.nrtdir, os.path.exists, os.listdir)

    def exists_db(self, time1, time2, net, sta, cha, loc):
        if self.__cursor is False:
            return False

        return self.__is_iso(time1, time2, net, sta, self.__db_path_exists) or \
            self.__is_sds(time1, time2, net, sta, cha, loc, self.archdir, self.__db_path_exists, self.__db_listdir) or \
            self.__is_sds(time1, time2, net, sta, cha, loc, self.nrtdir, self.__db_path_exists, self.__db_listdir)

    def iterdata(self, time1, time2, net, sta, cha, loc):
        lasttime = None
        lastrecord = None
        recstream = []

        if self.__is_iso(time1, time2, net, sta, os.path.exists):
            recstream.append(("isoarchive", self.isodir))
        if self.__is_sds(time1, time2, net, sta, cha, loc, self.archdir, os.path.exists, os.listdir):
            recstream.append(("sdsarchive", self.archdir))
        if self.__is_sds(time1, time2, net, sta, cha, loc, self.nrtdir, os.path.exists, os.listdir):
            recstream.append(("sdsarchive", self.nrtdir))
        
        if not recstream and self.exists_db(time1, time2, net, sta, cha, loc):
            raise TemporaryUnavailabilityException

        for (service, source) in recstream:
            if lastrecord:
                try:
                    etime = lastrecord.endTime()
                except ValueError:
                    logs.warning("SDS: record.endTime() raises Core.ValueException! Resulting SEED file maybe incorrect!")
                    etime = lastrecord.startTime()
                timetuple = time.strptime(etime.toString("%Y-%m-%d %H:%M:%S"), "%Y-%m-%d %H:%M:%S")                
                lasttime = datetime.datetime(*timetuple[:6])+datetime.timedelta(seconds=1) # avoids dublettes
                if lasttime >= time2:
                    break
                time1 = lasttime
                lastrecord = None
                
            self._recstream = IO.RecordStream.Create(service)
            if not self._recstream:
                logs.error("Could not fetch recordstream service '%s'" % service)
                raise StopIteration
            
            if not self._recstream.setSource(source):
                logs.error("Could not set recordstream source '%s'" % source)
                self._recstream = None
                raise StopIteration

            logs.debug("%s %s: addStream for %s-%s" % (service, source, str(time1), str(time2)))
            self._recstream.addStream(net,sta,loc,cha,Core.Time.FromString(str(time1),"%Y-%m-%d %H:%M:%S"),
                                      Core.Time.FromString(str(time2),"%Y-%m-%d %H:%M:%S"))

            try:
                recinput = IO.RecordInput(self._recstream, Core.Array.DOUBLE, Core.Record.SAVE_RAW)
                record = recinput.next()
                
                while record:
                    yield record.raw().str()
                    lastrecord = record
                    record = recinput.next()                

            except Exception, e:
                logs.error("SDS: Unexpected exception occured: %s" % e)
           

    def getwin(self, time1, time2, net, sta, cha, loc):
        """
        Returns the Mini SEED data records according to the given parameters.
        @arguments: time1, a datetime object specifying the start of time window
                    time2, a datetime object specifying the end of time window
                    net,   a string defining the network code
                    sta,   a string defining the station code
                    cha,   a string defining the channel code
                    loc,   a string defining the location ID
        @return: Mini SEED data records according to the given parameters
                 None otherwise
        """
        data = ""
        for rawrec in self.iterdata(time1, time2, net, sta, cha, loc):
            data += rawrec

        if not data:
            return None

        return data



def main():
    from seiscomp3 import Logging
    Logging.enableConsoleLogging(Logging.getGlobalChannel("debug"))
    Logging.enableConsoleLogging(Logging.getGlobalChannel("info"))
    Logging.enableConsoleLogging(Logging.getGlobalChannel("warning"))
    Logging.enableConsoleLogging(Logging.getGlobalChannel("error"))
    
    parser = OptionParser(usage="usage: %prog -n <nrt_directory> -a <cur_arch_directory> -i <iso_directory>")
    parser.add_option("-n", "--nrtdir", type="string", dest="nrtdir", help="nrt directory")   
    parser.add_option("-a", "--archdir", type="string", dest="archdir", help="current archive directory")
    parser.add_option("-i", "--isodir", type="string", dest="isodir", help="iso archive directory")
    (options, args) = parser.parse_args()
    
    if options.nrtdir is None:
        parser.error("Error: nrt directory not specified!")
    if options.archdir is None:
        parser.error("Error: sds archive directory not specified!")
    if options.isodir is None:
        parser.error("Error: iso archive directory not specified!")
                
    sdsobj = SDS(options.nrtdir,options.archdir,options.isodir)
    reqlist = [(datetime.datetime(2005,12,20,16,50,0),datetime.datetime(2006,1,30,2,8,0),"WM","EMAL","BHZ","")]
    count = 0
    for req in reqlist:
        count += 1
        print count,". request"
        data = sdsobj.getwin(*req)
        if data is not None:
            print "data_%d" % count
            fh = file("data_%d" % count,"w")
            fh.write(data)
            fh.close()
        else:
            print "No data available"
        

if __name__=="__main__":
    main()

