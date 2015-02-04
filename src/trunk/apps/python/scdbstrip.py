#!/usr/bin/env python

############################################################################
#    Copyright (C) 2009 by gempa GmbH                                      #
#                                                                          #
#    author: Jan Becker (gempa GmbH)                                       #
#    email:  jabe@gempa.de                                                 #
############################################################################

import re, sys, traceback
import seiscomp3.Client, seiscomp3.Utils

output = sys.stdout
error = sys.stderr

class RuntimeException(Exception):
  def __init__(self, what): self.what = what
  def __str__(self):
    return str(self.what)


class ExitRequestException(RuntimeException):
  def __init__(self): pass
  def __str__(self):
    return "exit requested"

class QueryInterface:
  def __init__(self, database):
    self._database = database
  def cnvCol(self, col):
    return self._database.convertColumnName(col)
  def getTables(self):
    return []
  def deleteObjectQuery(self, *v):
    return ""
  def deleteJournalQuery(self, *v):
    return ""
  def childQuery(self, mode, *v):
    return ""
  def childJournalQuery(self, mode, *v):
    return ""

class MySQLDB(QueryInterface):
  def __init__(self, database):
    QueryInterface.__init__(self, database)
  def getTables(self):
    tmp_tables = []
    if self._database.beginQuery("show tables") == False:
         return tmp_tables

    while self._database.fetchRow():
      tmp_tables.append(self._database.getRowFieldString(0))

    self._database.endQuery()
    return tmp_tables

  def deleteObjectQuery(self, *v):
    if v[0]:
      q = "delete " + v[0] + " from " + ", ".join(v) + " where " + \
          v[0] + "._oid=" + v[1] + "._oid and "
    else:
      q = "delete " + v[1] + " from " + ", ".join(v[1:]) + " where "

    for i in range(1, len(v)-1):
      if i > 1: q += " and "
      q += v[i] + "._oid=" + v[i+1] + "._oid"

    return q


  def deleteJournalQuery(self, *v):
    q = "delete JournalEntry from JournalEntry, " + ", ".join(v) + " where " + \
        v[0] + "._oid=" + v[1] + "._oid"

    for i in range(1, len(v)-1):
      q += " and " + v[i] + "._oid=" + v[i+1] + "._oid"

    q += " and JournalEntry.objectID=PublicObject.publicID"

    return q


  def childQuery(self, mode, *v):
    if v[0]:
      if   mode == "delete": q = "delete " + v[0]
      elif mode == "count" : q = "select count(*)"
      else: return ""

      q += " from " + ", ".join(v) + " where " + \
            v[0] + "._oid=" + v[1] + "._oid and "
    else:
      if   mode == "delete": q = "delete " + v[1]
      elif mode == "count" : q = "select count(*)"
      else: return ""

      q += " from " + ", ".join(v[1:]) + " where "

    for i in range(1, len(v)-1):
      if i > 1: q += " and "
      q += v[i] + "._parent_oid=" + v[i+1] + "._oid"

    return q


  def childJournalQuery(self, mode, *v):
    if v[0]:
      if   mode == "delete": q = "delete JournalEntry"
      elif mode == "count" : q = "select count(*)"
      else: return ""

      q += " from JournalEntry, " + ", ".join(v) + " where " + \
            v[0] + "._oid=" + v[1] + "._oid and "
    else:
      if   mode == "delete": q = "delete " + v[1]
      elif mode == "count" : q = "select count(*)"
      else: return ""

      q += " from JournalEntry, " + ", ".join(v[1:]) + " where "

    for i in range(1, len(v)-1):
      if i > 1: q += " and "
      q += v[i] + "._parent_oid=" + v[i+1] + "._oid"

    q += " and JournalEntry.objectID=PublicObject.publicID"
    return q

class PostgresDB(QueryInterface):
  def __init__(self, database):
    QueryInterface.__init__(self, database)
  def getTables(self):
    tmp_tables = []
    if self._database.beginQuery("SELECT table_name FROM information_schema.tables WHERE table_type = 'BASE TABLE' AND table_schema NOT IN ('pg_catalog', 'information_schema');") == False:
      return tmp_tables

    while self._database.fetchRow():
      tmp_tables.append(self._database.getRowFieldString(0))

    self._database.endQuery()
    return tmp_tables

  def deleteObjectQuery(self, *v):
    if v[0]:
      q = "delete from " + v[0] + " using " + ", ".join(v[1:]) + " where " + \
          v[0] + "._oid=" + v[1] + "._oid and "
    else:
      q = "delete from " + v[1] + " using " + ", ".join(v[2:]) + " where "

    for i in range(1, len(v)-1):
      if i > 1: q += " and "
      q += v[i] + "._oid=" + v[i+1] + "._oid"

    return q


  def deleteJournalQuery(self, *v):
    q = "delete from JournalEntry using " + ", ".join(v) + " where " + \
        v[0] + "._oid=" + v[1] + "._oid"

    for i in range(1, len(v)-1):
      q += " and " + v[i] + "._oid=" + v[i+1] + "._oid"

    q += " and JournalEntry." + self.cnvCol("objectID") + "=PublicObject." + self.cnvCol("publicID")

    return q


  def childQuery(self, mode, *v):
    if v[0]:
      if   mode == "delete": q = "delete from " + v[0] + " using " + ", ".join(v[1:])
      elif mode == "count" : q = "select count(*) from " + ", ".join(v)
      else: return ""

      q += " where " + \
            v[0] + "._oid=" + v[1] + "._oid and "
    else:
      if   mode == "delete": q = "delete from " + v[1] + " using " + ", ".join(v[2:])
      elif mode == "count" : q = "select count(*) from " + ", ".join(v[1:])
      else: return ""

      q += " where "

    for i in range(1, len(v)-1):
      if i > 1: q += " and "
      q += v[i] + "._parent_oid=" + v[i+1] + "._oid"

    return q

  def childJournalQuery(self, mode, *v):
    if v[0]:
      if   mode == "delete": q = "delete from JournalEntry using "
      elif mode == "count" : q = "select count(*) from "
      else: return ""

      q += ", ".join(v) + " where " + \
            v[0] + "._oid=" + v[1] + "._oid and "
    else:
      if   mode == "delete": q = "delete from " + v[1] + " using "
      elif mode == "count" : q = "select count(*) from "
      else: return ""

      q += " JournalEntry, " + ", ".join(v[1:]) + " where "

    for i in range(1, len(v)-1):
      if i > 1: q += " and "
      q += v[i] + "._parent_oid=" + v[i+1] + "._oid"

    q += " and JournalEntry." + self.cnvCol("objectID") + "=PublicObject." + self.cnvCol("publicID")
    return q

class DBCleaner(seiscomp3.Client.Application):
  def __init__(self, argc, argv):
    seiscomp3.Client.Application.__init__(self, argc, argv)

    self.setMessagingEnabled(False)
    self.setDatabaseEnabled(True, True)
    self.setDaemonEnabled(False)

    self._daysToKeep = 30
    self._hoursToKeep = 0
    self._minutesToKeep = 0
    self._datetime = None
    self._invertMode = False

    self._steps = 0
    self._currentStep = 0
    self._keepEvents = []

    self._timer = seiscomp3.Utils.StopWatch()

  def createCommandLineDescription(self):
    try:
      try:
        self.commandline().addGroup("Settings")
        self.commandline().addIntOption("Settings", "days", "Specify the number of days to keep")
        self.commandline().addIntOption("Settings", "hours", "Specify the number of hours to keep")
        self.commandline().addIntOption("Settings", "minutes", "Specify the number of minutes to keep")
        self.commandline().addStringOption("Settings", "datetime", "Specify the datetime (UTC) from which to keep all events in format [%Y-%m-%d %H:%M:%S]")
        self.commandline().addOption("Settings", "invert,i", "Delete all events after the specified time period")
        self.commandline().addStringOption("Settings", "keep-events", "Event-IDs to keep in the database")

        self.commandline().addGroup("Mode")
        self.commandline().addOption("Mode", "check", "Checks if unreachable objects exists")
        self.commandline().addOption("Mode", "clean-unused", "Removes all unreachable objects when in checkmode (default: off)")

      except:
        seiscomp3.Logging.warning("caught unexpected error %s" % sys.exc_info())
      return True
    except:
      info = traceback.format_exception(*sys.exc_info())
      for i in info: sys.stderr.write(i)
      sys.exit(-1)


  def initConfiguration(self):
    try:
      if not seiscomp3.Client.Application.initConfiguration(self): return False

      try: self._daysToKeep = self.configGetInt("database.cleanup.keep.days")
      except: pass
      try: self._hoursToKeep = self.configGetInt("database.cleanup.keep.hours")
      except: pass
      try: self._minutesToKeep = self.configGetInt("database.cleanup.keep.minutes")
      except: pass

      return True
    except:
      info = traceback.format_exception(*sys.exc_info())
      for i in info: sys.stderr.write(i)
      sys.exit(-1)


  def validateParameters(self):
    if not seiscomp3.Client.Application.validateParameters(self): return False

    try:
      try: self._daysToKeep = self.commandline().optionInt("days")
      except: pass
      try: self._hoursToKeep = self.commandline().optionInt("hours")
      except: pass
      try: self._minutesToKeep = self.commandline().optionInt("minutes")
      except: pass
      try: self._invertMode = self.commandline().hasOption("invert")
      except: pass
      try:
        eventIDs = self.commandline().optionString("keep-events")
        self._keepEvents = [id.strip() for id in eventIDs.split(',')]
      except: pass

      try:
        date = seiscomp3.Core.Time()
        if date.fromString(self.commandline().optionString("datetime"), "%Y-%m-%d %H:%M:%S") == True:
          error.write("using datetime option: %s\n" % date.toString("%Y-%m-%d %H:%M:%S"))
          self._datetime = date
        else:
          error.write("ERROR: datetime has wrong format\n")
          return False
      except: pass

      return True
    except:
      info = traceback.format_exception(*sys.exc_info())
      for i in info: sys.stderr.write(i)
      sys.exit(-1)


  def run(self):
    classname = self.database().className()
    if re.search('postgres', classname, re.IGNORECASE):
      self._query=PostgresDB(self.database())
    elif re.search('mysql', classname, re.IGNORECASE):
      self._query=MySQLDB(self.database())
    else:
      output.write("Error: Database interface %s is not supported\n" % (classname))
      output.flush()
      return False

    try:
      self._timer.restart()

      if self.commandline().hasOption("check"):
        return self.check()

      return self.clean()
    except:
      info = traceback.format_exception(*sys.exc_info())
      for i in info: sys.stderr.write(i)
      sys.exit(-1)


  def checkTable(self, table):
    self.runCommand("update tmp_object set used=1 where _oid in (select _oid from %s)" % table)


  def check(self):
    try:
      if self._datetime is None:
        timeSpan = seiscomp3.Core.TimeSpan(self._daysToKeep*24*3600 + self._hoursToKeep*3600 + self._minutesToKeep*60)
        # All times are given in localtime
        timestamp = seiscomp3.Core.Time.LocalTime() - timeSpan
      else:
        timestamp = self._datetime

      output.write("[INFO] Check objects older than %s\n" % timestamp.toString("%Y-%m-%d %H:%M:%S"))

      tables = self._query.getTables()
      if len(tables) == 0:
         return False

      if "Object" in tables: tables.remove("Object")
      if "object" in tables: tables.remove("object")
      if "PublicObject" in tables: tables.remove("PublicObject")
      if "publicobject" in tables: tables.remove("publicobject")
      if "Meta" in tables: tables.remove("Meta")
      if "meta" in tables: tables.remove("meta")

      self._steps = len(tables) + 1

      if self.commandline().hasOption("clean-unused"):
        self._steps = self._steps + 1

      # Skip the first 5 objects id' that are reserved for metaobjects (Config,
      # QualityControl, inventory, EventParameters, routing)
      tmp_object = "\
      create temporary table tmp_object as \
        select _oid, 0 as used from Object where _oid > 5 and _timestamp < '%s'\
      " % timestamp.toString("%Y-%m-%d %H:%M:%S")

      self.beginMessage("Search objects")
      if self.runCommand(tmp_object) == False: return False
      self.endMessage(self.globalCount("tmp_object"))

      for table in tables:
        self.beginMessage("Check table %s" % table)
        self.checkTable(table)
        self.endMessage(self.usedCount("tmp_object"))

      unusedObjects = self.unusedCount("tmp_object")

      if self.commandline().hasOption("clean-unused"):
        self.delete("Remove unreachable objects", self.deleteUnusedRawObjects, "tmp_object")

      self.beginMessage("%d unused objects found" % unusedObjects)
      if self.runCommand("drop table tmp_object") == False: return False
      self.endMessage()

      return True

    except RuntimeException, e:
      error.write("\nException: %s\n" % str(e))
      return False

    except:
      info = traceback.format_exception(*sys.exc_info())
      for i in info: sys.stderr.write(i)
      sys.exit(-1)


  def clean(self):
    try:
      if self._datetime is None:
        timeSpan = seiscomp3.Core.TimeSpan(self._daysToKeep*24*3600 + self._hoursToKeep*3600 + self._minutesToKeep*60)
        # All times are given in GMT (UTC)
        timestamp = seiscomp3.Core.Time.GMT() - timeSpan
      else:
        timestamp = self._datetime

      if not self._invertMode:
        output.write("[INFO] Keep objects after %s\n" % timestamp.toString("%Y-%m-%d %H:%M:%S"))
      else:
        output.write("[INFO] Keep objects before %s\n" % timestamp.toString("%Y-%m-%d %H:%M:%S"))

      if len(self._keepEvents) > 0:
        output.write("[INFO] Keep events in db: %s\n" % ",".join(self._keepEvents))

      op = '<';
      if self._invertMode: op = '>='

      old_events = "\
      create temporary table old_events as \
        select Event._oid, PEvent.%s \
        from Event, PublicObject as PEvent, Origin, PublicObject as POrigin \
        where Event._oid=PEvent._oid and \
              Origin._oid=POrigin._oid and \
              Event.%s=POrigin.%s and \
              Origin.%s %s '%s'\
      " % (self.cnvCol("publicID"), self.cnvCol("preferredOriginID"), self.cnvCol("publicID"), self.cnvCol("time_value"), op, timestamp.toString("%Y-%m-%d %H:%M:%S"))

      if len(self._keepEvents) > 0:
        old_events += " and PEvent." + self.cnvCol("publicID") + " not in ('%s')" % "','".join(self._keepEvents)

      self._steps = 32

      self.beginMessage("Find old events")
      if self.runCommand(old_events) == False: return False
      self.endMessage(self.globalCount("old_events"))

      # Delete OriginReferences of old events
      self.delete("Delete origin references of old events", self.deleteChilds, "OriginReference", "old_events")

      # Delete FocalMechanismReference of old events
      self.delete("Delete focal mechanism references of old events", self.deleteChilds, "FocalMechanismReference", "old_events")

      # Delete EventDescription of old events
      self.delete("Delete event descriptions of old events", self.deleteChilds, "EventDescription", "old_events")

      # Delete Comments of old events
      self.delete("Delete comments of old events", self.deleteChilds, "Comment", "old_events")

      # Delete old events
      self.delete("Delete old events", self.deleteObjects, "Event", "old_events")

      self.beginMessage("Cleaning up temporary results")
      if self.runCommand("drop table old_events") == False: return False
      self.endMessage()

      tmp_fm = "\
      create temporary table tmp_fm as \
        select FocalMechanism._oid, PFM.%s, 0 as used \
        from PublicObject as PFM, FocalMechanism \
        where PFM._oid=FocalMechanism._oid\
      " % (self.cnvCol("publicID"))

      self.beginMessage("Find unassociated focal mechanisms")

      if self.runCommand(tmp_fm) == False: return False

      tmp_fm = "\
      update tmp_fm set used=1 \
      where " + self.database().convertColumnName("publicID") + " in (select distinct " + self.database().convertColumnName("focalMechanismID") + " from FocalMechanismReference) \
      "

      if self.runCommand(tmp_fm) == False: return False

      self.endMessage(self.unusedCount("tmp_fm"))

      # Delete Comments of unassociated focal mechanisms
      self.delete("Delete comments of unassociation focal mechanisms", self.deleteUnusedChilds, "Comment", "tmp_fm")

      # Delete MomentTensor.Comments of unassociated focal mechanisms
      self.delete("Delete moment tensor comments of unassociated focal mechanisms", self.deleteUnusedChilds, "Comment", "MomentTensor", "tmp_fm")

      # Delete MomentTensor.DataUsed of unassociated focal mechanisms
      self.delete("Delete moment tensor data of unassociated focal mechanisms", self.deleteUnusedChilds, "DataUsed", "MomentTensor", "tmp_fm")

      # Delete MomentTensor.PhaseSetting of unassociated focal mechanisms
      self.delete("Delete moment tensor phase settings of unassociated focal mechanisms", self.deleteUnusedChilds, "MomentTensorPhaseSetting", "MomentTensor", "tmp_fm")

      # Delete MomentTensor.StationContribution.ComponentContribution of unassociated focal mechanisms
      self.delete("Delete moment tensor component contributions of unassociated focal mechanisms", self.deleteUnusedChilds, "MomentTensorComponentContribution", "MomentTensorStationContribution", "MomentTensor", "tmp_fm")

      # Delete MomentTensor.StationContributions of unassociated focal mechanisms
      self.delete("Delete moment tensor station contributions of unassociated focal mechanisms", self.deleteUnusedPublicChilds, "MomentTensorStationContribution", "MomentTensor", "tmp_fm")

      # Delete MomentTensors of unassociated focal mechanisms
      self.delete("Delete moment tensors of unassociated focal mechanisms", self.deleteUnusedPublicChilds, "MomentTensor", "tmp_fm")

      # Delete FocalMechanism itself
      self.delete("Delete unassociated focal mechanisms", self.deleteUnusedObjects, "FocalMechanism", "tmp_fm")

      self.beginMessage("Cleaning up temporary results")
      if self.runCommand("drop table tmp_fm") == False: return False
      self.endMessage()

      tmp_origin = "\
      create temporary table tmp_origin as \
        select Origin._oid, %s, 0 as used \
        from PublicObject, Origin \
        where PublicObject._oid=Origin._oid and \
              Origin.%s %s '%s'\
      " % (self.cnvCol("publicID"), self.cnvCol("time_value"), op, timestamp.toString("%Y-%m-%d %H:%M:%S"))

      self.beginMessage("Find unassociated origins")

      if self.runCommand(tmp_origin) == False: return False

      tmp_origin = "\
      update tmp_origin set used=1 \
      where (" + self.database().convertColumnName("publicID") + " in (select distinct " + self.database().convertColumnName("originID") + " from OriginReference)) \
      or (" + self.database().convertColumnName("publicID") + " in (select " + self.database().convertColumnName("derivedOriginID") + " from MomentTensor))"

      if self.runCommand(tmp_origin) == False: return False

      self.endMessage(self.unusedCount("tmp_origin"))


      # Delete Arrivals of unassociated origins
      self.delete("Delete unassociated arrivals", self.deleteUnusedChilds, "Arrival", "tmp_origin")

      # Delete StationMagnitudes of unassociated origins
      self.delete("Delete unassociated station magnitudes", self.deleteUnusedPublicChilds, "StationMagnitude", "tmp_origin")

      # Delete StationMagnitudeContributions of unassociated origins
      self.delete("Delete unassociated station magnitude contributions", self.deleteUnusedChilds, "StationMagnitudeContribution", "Magnitude", "tmp_origin")

      # Delete Magnitudes of unassociated origins
      self.delete("Delete unassociated magnitudes", self.deleteUnusedPublicChilds, "Magnitude", "tmp_origin")

      # Delete Comments of unassociated origins
      self.delete("Delete comments of unassociation origins", self.deleteUnusedChilds, "Comment", "tmp_origin")

      # Delete CompositeTimes of unassociated origins
      self.delete("Delete composite times of unassociation origins", self.deleteUnusedChilds, "CompositeTime", "tmp_origin")

      # Delete Origins itself
      self.delete("Delete unassociated origins", self.deleteUnusedObjects, "Origin", "tmp_origin")

      self.beginMessage("Cleaning up temporary results")
      if self.runCommand("drop table tmp_origin") == False: return False
      self.endMessage()

      # Delete all unassociated picks (via arrivals)

      self.beginMessage("Find unassociated picks")

      tmp_pick = "\
      create temporary table tmp_pick as \
        select Pick._oid, %s, 0 as used \
        from PublicObject, Pick \
        where PublicObject._oid=Pick._oid and \
              Pick.%s %s '%s' \
      " % (self.cnvCol("publicID"), self.cnvCol("time_value"), op, timestamp.toString("%Y-%m-%d %H:%M:%S"))

      if self.runCommand(tmp_pick) == False: return False

      tmp_pick = "\
      update tmp_pick set used=1 \
      where "+ self.cnvCol("publicID") + " in \
        (select distinct " + self.cnvCol("pickID") + " from Arrival) \
      "

      if self.runCommand(tmp_pick) == False: return False

      self.endMessage(self.unusedCount("tmp_pick"))

      self.delete("Delete unassociated picks", self.deleteUnusedObjects, "Pick", "tmp_pick")

      self.beginMessage("Cleaning up temporary results")
      if self.runCommand("drop table tmp_pick") == False: return False
      self.endMessage()


      # Delete all unassociated amplitudes (via stationmagnitudes)

      self.beginMessage("Find unassociated amplitudes")

      tmp_amp = "\
      create temporary table tmp_amp as \
        select Amplitude._oid, " + self.cnvCol("publicID") + ", 0 as used \
        from PublicObject, Amplitude \
        where PublicObject._oid=Amplitude._oid and \
              Amplitude." + self.cnvCol("timeWindow_reference") + " %s '%s' \
      " % (op, timestamp.toString("%Y-%m-%d %H:%M:%S"))

      if self.runCommand(tmp_amp) == False: return False

      tmp_amp = "\
      update tmp_amp set used=1 \
      where " + self.cnvCol("publicID") + " in \
        (select distinct " + self.cnvCol("amplitudeID") + " from StationMagnitude) \
      "

      if self.runCommand(tmp_amp) == False: return False

      self.endMessage(self.unusedCount("tmp_amp"))

      self.delete("Delete unassociated station amplitudes", self.deleteUnusedObjects, "Amplitude", "tmp_amp")

      self.beginMessage("Cleaning up temporary results")
      if self.runCommand("drop table tmp_amp") == False: return False
      self.endMessage()

      self.beginMessage("Delete waveform quality parameters")
      if self.runCommand(self._query.deleteObjectQuery("Object", "WaveformQuality") + \
                         "WaveformQuality.%s %s '%s'" % (self.cnvCol("end"), op, timestamp.toString("%Y-%m-%d %H:%M:%S"))) == False: return False
      if self.runCommand("delete from WaveformQuality where WaveformQuality.%s %s '%s'" % (self.cnvCol("end"), op, timestamp.toString("%Y-%m-%d %H:%M:%S"))) == False: return False
      self.endMessage()

      return True

    except RuntimeException, e:
      error.write("\nException: %s\n" % str(e))
      return False

    except:
      info = traceback.format_exception(*sys.exc_info())
      for i in info: sys.stderr.write(i)
      sys.exit(-1)


  def cnvCol(self, col):
    return self.database().convertColumnName(col)


  def beginMessage(self, msg):
    output.write("[%3d%%] " % (self._currentStep*100/self._steps))
    output.write(msg + "...")
    output.flush()
    self._currentStep = self._currentStep + 1


  def endMessage(self, count=None):
    if not count is None:
      output.write("done (%d)" % count)
    else:
      output.write("done")

    span = self._timer.elapsed().seconds()
    output.write(", time spent: %d %02d:%02d:%02d\n" % (span / 86400, (span % 86400) / 3600, (span % 3600) / 60, span % 60))


  def runCommand(self, q):
    if self.isExitRequested():
      raise ExitRequestException

    if self.database().execute(q) == False:
      raise RuntimeException("ERROR: command '%s' failed\n" % q)
      return False

    if self.isExitRequested():
      raise ExitRequestException

    return True


  def runQuery(self, q):
    if self.isExitRequested():
      raise ExitRequestException

    count = "-1"

    if self.database().beginQuery(q) == False:
      raise RuntimeException("ERROR: command '%s' failed\n" % q)
      return [count]

    if self.database().fetchRow():
      count = self.database().getRowFieldString(0)

    self.database().endQuery()

    if self.isExitRequested():
      raise ExitRequestException

    return [count]


  def globalCount(self, table):
    return int(self.runQuery("select count(*) from %s" % table)[0])


  def usedCount(self, table):
    return int(self.runQuery("select count(*) from %s where used=1" % table)[0])


  def unusedCount(self, table):
    return int(self.runQuery("select count(*) from %s where used=0" % table)[0])

  def deleteChilds(self, *v):
    count = int(self.runQuery(self._query.childQuery("count", "Object", *v))[0])
    self.runCommand(self._query.childQuery("delete", "Object", *v))
    self.runCommand(self._query.childQuery("delete", None, *v))
    return count


  def deleteUnusedChilds(self, *v):
    count = int(self.runQuery(self._query.childQuery("count", "Object", *v) + " and used=0")[0])
    self.runCommand(self._query.childQuery("delete", "Object", *v) + " and used=0")
    self.runCommand(self._query.childQuery("delete", None, *v) + " and used=0")
    return count


  def deleteUnusedPublicChilds(self, *v):
    count = int(self.runQuery(self._query.childQuery("count", "Object", *v) + " and used=0")[0])
    self.runCommand(self._query.childJournalQuery("delete", "PublicObject", *v) + " and used=0")
    self.runCommand(self._query.childQuery("delete", "Object", *v) + " and used=0")
    self.runCommand(self._query.childQuery("delete", "PublicObject", *v) + " and used=0")
    self.runCommand(self._query.childQuery("delete", None, *v) + " and used=0")
    return count


  def deleteUnusedRawObjects(self, *v):
    self.runCommand(self._query.deleteJournalQuery("PublicObject", *v) + " and used=0")
    self.runCommand(self._query.deleteObjectQuery(None, "Object", *v) + " and used=0")
    self.runCommand(self._query.deleteObjectQuery(None, "PublicObject", *v) + " and used=0")
    return None

  def deleteObjects(self, *v):
    self.runCommand(self._query.deleteJournalQuery("PublicObject", *v))
    self.runCommand(self._query.deleteObjectQuery("Object", *v))
    self.runCommand(self._query.deleteObjectQuery("PublicObject", *v))
    self.runCommand(self._query.deleteObjectQuery(None, *v))
    return None

  def deleteUnusedObjects(self, *v):
    self.runCommand(self._query.deleteJournalQuery("PublicObject", *v) + " and used=0")
    self.runCommand(self._query.deleteObjectQuery("Object", *v) + " and used=0")
    self.runCommand(self._query.deleteObjectQuery("PublicObject", *v) + " and used=0")
    self.runCommand(self._query.deleteObjectQuery(None, *v) + " and used=0")
    return None

  def delete(self, message, func, *v):
    self.beginMessage(message)
    count = func(*v)
    self.endMessage(count)
    return count


app = DBCleaner(len(sys.argv), sys.argv)
sys.exit(app())
