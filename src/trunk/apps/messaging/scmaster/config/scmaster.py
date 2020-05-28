from __future__ import print_function

import os
import sys
import subprocess
import seiscomp3.Kernel

#------------------------------------------------------------------------------
# Python version depended string conversion
if sys.version_info[0] < 3:
    py3bstr = str
    py3ustr = str

else:
    py3bstr = lambda s: s.encode('utf-8')
    py3ustr = lambda s: s.decode('utf-8', 'replace')

def check_output(cmd):
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, shell=True)
    out = proc.communicate()
    return [py3ustr(out[0]), py3ustr(out[1]), proc.returncode]


def createMYSQLDB(db, rwuser, rwpwd, rouser, ropwd, rwhost, rootpwd, drop, schemapath):
    cmd = "mysql -u root -h " + rwhost
    if rootpwd:
        cmd += " -p" + rootpwd

    sys.stdout.write("+ Create MYSQL database\n")

    out = check_output(cmd + " -s --skip-column-names -e \"select version()\"")
    version = out[0].strip()
    err = out[1]
    if out[1]:
        sys.stdout.write("  %s\n" % out[1].strip())
    if out[2]:
        err = "Exitcode: %d" % out[2]
        sys.stdout.write("  %s\n"
                         "  Could not determine MYSQL server version. Is the root password correct\n"
                         "  and MYSQL is running and the client installed on this machine?\n" % err.strip())
    sys.stdout.write("  + Found MYSQL server version %s\n" % version)

    if drop:
        q = "DROP DATABASE IF EXISTS %s;" % db
        sys.stdout.write("  + Drop database %s\n" % db)
        out = check_output(cmd + " -e \"%s\"" % q)
        if out[1]:
            sys.stdout.write("  %s\n" % out[1].strip())
        if out[2]:
            if not out[1]:
                sys.stdout.write("  Returned with error: %d\n" % out[2])
            return False

    sys.stdout.write("  + Create database %s\n" % db)
    q = "CREATE DATABASE %s CHARACTER SET utf8 COLLATE utf8_bin;" % db
    out = check_output(cmd + " -e \"%s\"" % q)
    if out[1]:
        sys.stdout.write("  %s\n" % out[1].strip())
    if out[2]:
        if not out[1]:
            sys.stdout.write("  Returned with error: %d\n" % out[2])
        return False

    sys.stdout.write("  + Setup user roles\n")
    # MySQL 8 requires explicit "CREATE USER", but this fails
    # if the users already exists.
    # "CREATE USER IF NOT EXISTS" is not supported by MySQL<5.7.
    # Drop possibly existing users, ignoring errors.
    check_output(cmd + " -e \"DROP USER '%s'@'localhost';\"" % rwuser)
    check_output(cmd + " -e \"DROP USER '%s'@'%%';\"" % rwuser)
    q = "CREATE USER '%s'@'localhost' IDENTIFIED BY '%s';" % (
        rwuser, rwpwd)
    q += "GRANT ALL ON %s.* TO '%s'@'localhost';" % (
        db, rwuser)
    q += "CREATE USER '%s'@'%%' IDENTIFIED BY '%s';" % (
        rwuser, rwpwd)
    q += "GRANT ALL ON %s.* TO '%s'@'%%';" % (
        db, rwuser)

    if rwuser != rouser:
        check_output(cmd + " -e \"DROP USER '%s'@'localhost';\"" % rouser)
        check_output(cmd + " -e \"DROP USER '%s'@'%%';\"" % rouser)
        q += "CREATE USER '%s'@'localhost' IDENTIFIED BY '%s';" % (
            rouser, ropwd)
        q += "GRANT SELECT ON %s.* TO '%s'@'localhost';" % (
            db, rouser)
        q += "CREATE USER '%s'@'%%' IDENTIFIED BY '%s';" % (
            rouser, ropwd)
        q += "GRANT SELECT ON %s.* TO '%s'@'%%';" % (
            db, rouser)

    out = check_output(cmd + " -e \"%s\"" % q)
    if out[1]:
        sys.stdout.write("  %s\n" % out[1].strip())
    if out[2]:
        if not out[1]:
            sys.stdout.write("  Returned with error: %d\n" % out[2])
        return False

    sys.stdout.write("  + Create tables\n")
    q = "USE %s; source %s;" % (db, os.path.join(schemapath, "mysql.sql"))
    out = check_output(cmd + " -e \"%s\"" % q)
    if out[1]:
        sys.stdout.write("  %s\n" % out[1].strip())
    if out[2]:
        if not out[1]:
            sys.stdout.write("  Returned with error: %d\n" % out[2])
        return False

    return True


# The kernel module which starts scmaster if enabled
class Module(seiscomp3.Kernel.CoreModule):
    def __init__(self, env):
        seiscomp3.Kernel.CoreModule.__init__(
            self, env, env.moduleName(__file__))
        # High priority
        self.order = -1

        # Default values
        self.messaging = True
        self.messagingPort = 4803

        try:
            self.messaging = self.env.getBool("messaging.enable")
        except:
            pass
        try:
            self.messagingPort = self.env.getInt("messaging.port")
        except:
            pass

    # Add master port
    def _get_start_params(self):
        return seiscomp3.Kernel.Module._get_start_params(self) + \
               " -H localhost:%d" % self.messagingPort

    def start(self):
        if not self.messaging:
            print("[kernel] %s is disabled by config" % self.name)
            return 0

        return seiscomp3.Kernel.CoreModule.start(self)

    def check(self):
        if not self.messaging:
            print("[kernel] %s is disabled by config" % self.name)
            return 0

        return seiscomp3.Kernel.CoreModule.check(self)

    def status(self, shouldRun):
        if not self.messaging:
            shouldRun = False
        return seiscomp3.Kernel.CoreModule.status(self, shouldRun)

    def setup(self, setup_config):
        cfgfile = os.path.join(self.env.SEISCOMP_ROOT,
                               "etc", self.name + ".cfg")
        schemapath = os.path.join(self.env.SEISCOMP_ROOT, "share", "db")

        cfg = seiscomp3.Config.Config()
        cfg.readConfig(cfgfile)
        try:
            dbenable = setup_config.getBool(self.name + ".database.enable")
        except:
            sys.stderr.write("  - database.enable not set, ignoring setup\n")
            return 0

        dbBackend = None

        if not dbenable:
            cfg.setString("plugins", "")
        else:
            try:
                dbBackend = setup_config.getString(
                    self.name + ".database.enable.backend")
            except:
                sys.stderr.write(
                    "  - database backend not set, ignoring setup\n")
                return 1

            cfg.setString("plugins", "dbplugin")
            # Configure db backend for scmaster
            cfg.setString("core.plugins", "db" + dbBackend)

            try:
                db = setup_config.getString(self.name + ".database.enable.db")
            except:
                sys.stderr.write("  - database name not set, ignoring setup\n")
                return 1

            try:
                rwhost = setup_config.getString(
                    self.name + ".database.enable.rwhost")
            except:
                sys.stderr.write(
                    "  - database host (rw) not set, ignoring setup\n")
                return 1

            try:
                rwuser = setup_config.getString(
                    self.name + ".database.enable.rwuser")
            except:
                sys.stderr.write(
                    "  - database user (rw) not set, ignoring setup\n")
                return 1

            try:
                rwpwd = setup_config.getString(
                    self.name + ".database.enable.rwpwd")
            except:
                sys.stderr.write(
                    "  - database password (rw) not set, ignoring setup\n")
                return 1

            try:
                rohost = setup_config.getString(
                    self.name + ".database.enable.rohost")
            except:
                sys.stderr.write(
                    "  - database host (ro) not set, ignoring setup\n")
                return 1

            try:
                rouser = setup_config.getString(
                    self.name + ".database.enable.rouser")
            except:
                sys.stderr.write(
                    "  - database user (ro) not set, ignoring setup\n")
                return 1

            try:
                ropwd = setup_config.getString(
                    self.name + ".database.enable.ropwd")
            except:
                sys.stderr.write(
                    "  - database password (ro) not set, ignoring setup\n")
                return 1

            if dbBackend == "mysql":
                try:
                    create = setup_config.getBool(
                        self.name + ".database.enable.backend.create")
                except:
                    create = False
                try:
                    drop = setup_config.getBool(
                        self.name + ".database.enable.backend.create.drop")
                except:
                    drop = False
                try:
                    rootpwd = setup_config.getString(
                        self.name + ".database.enable.backend.create.rootpw")
                except:
                    rootpwd = ""

                if create:
                    if not createMYSQLDB(db, rwuser, rwpwd, rouser, ropwd, rwhost, rootpwd, drop, schemapath):
                        sys.stdout.write("  - Failed to setup database\n")
                        return 1
                    #sys.stderr.write("Sorry, creation of the database is currently not supported\n")

            cfg.setString("plugins.dbPlugin.dbDriver", dbBackend)
            cfg.setString("plugins.dbPlugin.readConnection",
                          rouser + ":" + ropwd + "@" + rohost + "/" + db)
            cfg.setString("plugins.dbPlugin.writeConnection",
                          rwuser + ":" + rwpwd + "@" + rwhost + "/" + db)

        cfg.writeConfig()

        # Now we need to insert the corresponding plugin to etc/global.cfg
        # that all connected local clients can handle the database backend
        if dbBackend:
            cfgfile = os.path.join(self.env.SEISCOMP_ROOT, "etc", "global.cfg")
            cfg = seiscomp3.Config.Config()
            cfg.readConfig(cfgfile)
            cfg.setString("core.plugins", "db" + dbBackend)
            cfg.writeConfig()

        return 0

    def updateConfig(self):
        cfgfile = os.path.join(self.env.SEISCOMP_ROOT,
                               "etc", self.name + ".cfg")
        schemapath = os.path.join(self.env.SEISCOMP_ROOT, "share", "db")

        cfg = seiscomp3.Config.Config()
        cfg.readConfig(cfgfile)
        try:
            dbenable = "dbplugin" in cfg.getStrings("plugins")
        except:
            return 0

        if not dbenable:
            return 0

        try:
            backend = cfg.getString("plugins.dbPlugin.dbDriver")
        except:
            print("WARNING: DB plugin activated but no backend configured", file=sys.stderr)
            return 0

        if backend not in ("mysql", "postgresql"):
            print("WARNING: Only MySQL and PostgreSQL migrations are supported right now. Please check and "\
                  "upgrade the database schema version yourselves.", file=sys.stderr)
            return 0

        sys.stderr.write("  * check database write access ... ")
        sys.stderr.flush()

        # 1. Parse connection
        try:
            params = cfg.getString("plugins.dbPlugin.writeConnection")
        except:
            print("failed", file=sys.stderr)
            print("WARNING: DB plugin activated but not writeConnection configured", file=sys.stderr)
            return 0

        user = 'sysop'
        pwd = 'sysop'
        host = 'localhost'
        db = 'seiscomp3'

        tmp = params.split('@')
        if len(tmp) > 1:
            params = tmp[1]

            tmp = tmp[0].split(':')
            if len(tmp) == 1:
                user = tmp[0]
                pwd = None
            elif len(tmp) == 2:
                user = tmp[0]
                pwd = tmp[1]
            else:
                print("failed", file=sys.stderr)
                print("WARNING: Invalid scmaster.cfg:plugins.dbPlugin.writeConnection, cannot check schema version", file=sys.stderr)
                return 0

        tmp = params.split('/')
        if len(tmp) > 1:
            host = tmp[0]
            db = tmp[1]
        else:
            host = tmp[0]

        db = db.split('?')[0]

        # 2. Try to login

        if backend == "mysql":
            cmd = "mysql -u \"%s\" -h \"%s\" -D\"%s\" --skip-column-names" % (
                user, host, db)
            if pwd:
                cmd += " -p\"%s\"" % pwd
            cmd += " -e \"SELECT value from Meta where name='Schema-Version'\""
        else:
            if pwd:
                os.environ['PGPASSWORD'] = pwd
            cmd = "psql -U \"%s\" -h \"%s\" -t \"%s\"" % (user, host, db)
            cmd += " -c \"SELECT value from Meta where name='Schema-Version'\""

        out = check_output(cmd)
        if out[2] != 0:
            print("failed", file=sys.stderr)
            print("WARNING: mysql returned with error:", file=sys.stderr)
            print(out[1].strip(), file=sys.stderr)
            return 0

        print("OK", file=sys.stderr)

        version = out[0].strip()
        print("  * database schema version is %s" % version, file=sys.stderr)

        try:
            vmaj, vmin = [int(t) for t in version.split('.')]
        except:
            print("WARNING: wrong version format: expected MAJOR.MINOR", file=sys.stderr)
            return 0

        strictVersionCheck = True
        try:
            strictVersionCheck = cfg.getBool(
                "plugins.dbPlugin.strictVersionCheck")
        except:
            pass

        if not strictVersionCheck:
            print("  * database version check is disabled", file=sys.stderr)
            return 0

        migrations = os.path.join(schemapath, "migrations", backend)
        migration_paths = {}

        vcurrmaj = 0
        vcurrmin = 0

        for f in os.listdir(migrations):
            if os.path.isfile(os.path.join(migrations, f)):
                name, ext = os.path.splitext(f)
                if ext != '.sql':
                    continue
                try:
                    vfrom, vto = name.split('_to_')
                except:
                    continue

                try:
                    vfrommaj, vfrommin = [int(t) for t in vfrom.split('_')]
                except:
                    continue

                try:
                    vtomaj, vtomin = [int(t) for t in vto.split('_')]
                except:
                    continue

                migration_paths[(vfrommaj, vfrommin)] = (vtomaj, vtomin)

                if (vtomaj > vcurrmaj) or ((vtomaj == vcurrmaj) and (vtomin > vcurrmin)):
                    vcurrmaj = vtomaj
                    vcurrmin = vtomin

        print("  * last migration version is %d.%d" % (
            vcurrmaj, vcurrmin), file=sys.stderr)

        if vcurrmaj == vmaj and vcurrmin == vmin:
            print("  * schema up-to-date", file=sys.stderr)
            return 0

        if (vmaj, vmin) not in migration_paths:
            print("  * no migrations found", file=sys.stderr)
            return 0

        print("  * migration to the current version is required.  apply the following", file=sys.stderr)
        print("    scripts in exactly the given order:", file=sys.stderr)
        while (vmaj, vmin) in migration_paths:
            (vtomaj, vtomin) = migration_paths[(vmaj, vmin)]
            print("    * %s" % os.path.join(
                migrations, "%d_%d_to_%d_%d.sql" % (vmaj, vmin, vtomaj, vtomin)), file=sys.stderr)
            (vmaj, vmin) = (vtomaj, vtomin)

        return 1
