############################################################################
#    Copyright (C) by gempa GmbH, GFZ Potsdam                              #
#                                                                          #
#    You can redistribute and/or modify this program under the             #
#    terms of the SeisComP Public License.                                 #
#                                                                          #
#    This program is distributed in the hope that it will be useful,       #
#    but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#    SeisComP Public License for more details.                             #
############################################################################

import os
import sys
import time
import string
import subprocess
import seiscomp3.Config


class Template(string.Template):
    idpattern = r'[_a-z][_a-z0-9.]*'


class Environment(seiscomp3.Config.Config):
    def __init__(self, rootPath):
        seiscomp3.Config.Config.__init__(self)
        self.SEISCOMP_ROOT = rootPath
        try:
            self.home_dir = os.environ["HOME"]
        except:
            self.home_dir = "."

        try:
            self.local_config_dir = os.environ["SEISCOMP_LOCAL_CONFIG"]
        except:
            self.local_config_dir = os.path.join(self.home_dir, ".seiscomp3")

        self.root = rootPath
        self.bin_dir = os.path.join(self.root, "bin")
        self.data_dir = os.path.join(self.root, "share")
        self.etc_dir = os.path.join(self.root, "etc")
        self.etc_defaults_dir = os.path.join(self.root, "etc", "defaults")
        self.escriptions_dir = os.path.join(self.root, "etc", "descriptions")
        self.key_dir = os.path.join(self.root, "etc", "key")
        self.var_dir = os.path.join(self.root, "var")
        self.log_dir = os.path.join(self.local_config_dir, "log")
        self.cwd = None
        self.last_template_file = None

        self._csv = False
        self._readConfig()

        os.environ["SEISCOMP_ROOT"] = self.SEISCOMP_ROOT

        # Add LD_LIBRARY_PATH and PATH to OS environment
        LD_LIBRARY_PATH = os.path.join(self.SEISCOMP_ROOT, "lib")
        BIN_PATH = os.path.join(self.SEISCOMP_ROOT, "bin")
        SBIN_PATH = os.path.join(self.SEISCOMP_ROOT, "sbin")
        PATH = BIN_PATH + ":" + SBIN_PATH
        PYTHONPATH = os.path.join(self.SEISCOMP_ROOT, "lib", "python")
        try:
            LD_LIBRARY_PATH = os.environ["LD_LIBRARY_PATH"] + \
                ":" + LD_LIBRARY_PATH
        except:
            pass
        os.environ["LD_LIBRARY_PATH"] = LD_LIBRARY_PATH
        try:
            PATH = PATH + ":" + os.environ["PATH"]
        except:
            pass
        os.environ["PATH"] = PATH
        try:
            PYTHONPATH = os.environ["PYTHONPATH"] + ":" + PYTHONPATH
        except:
            pass
        os.environ["PYTHONPATH"] = PYTHONPATH

        # Create required directories
        try:
            os.makedirs(os.path.join(self.root, "var", "log"))
        except:
            pass

        try:
            os.makedirs(os.path.join(self.root, "var", "run"))
        except:
            pass


    def __del__(self):
        if self.cwd:
            os.chdir(self.cwd)

    def _readConfig(self):
        self.syslog = False

        # Read configuration file
        kernelCfg = os.path.join(self.root, "etc", "kernel.cfg")
        if self.readConfig(kernelCfg) == False:
            return

        try:
            self.syslog = self.getBool("syslog")
        except:
            pass

    # Changes into the SEISCOMP_ROOT directory
    def chroot(self):
        if self.root:
            # Remember current directory
            self.cwd = os.getcwd()
            os.chdir(self.SEISCOMP_ROOT)
            self.root = ""

    # Changes back to the current workdir
    def chback(self):
        if self.cwd:
            os.chdir(self.cwd)
            self.cwd = None
            self.root = self.SEISCOMP_ROOT

    def resolvePath(self, path):
        return path.replace("@LOGDIR@", self.log_dir)\
                   .replace("@CONFIGDIR@", self.local_config_dir)\
                   .replace("@DEFAULTCONFIGDIR@", self.etc_defaults_dir)\
                   .replace("@SYSTEMCONFIGDIR@", self.etc_dir)\
                   .replace("@ROOTDIR@", self.root)\
                   .replace("@DATADIR@", self.data_dir)\
                   .replace("@KEYDIR@", self.key_dir)\
                   .replace("@HOMEDIR@", self.home_dir)

    def setCSVOutput(self, csv):
        self._csv = csv

    def enableModule(self, name):
        runFile = os.path.join(self.root, "etc", "init", name + ".auto")
        if os.path.exists(runFile):
            print "%s is already enabled" % name
            return 0
        try:
            open(runFile, 'w').close()
            print "enabled %s" % name
            return 0
        except Exception, exc:
            sys.stderr.write(str(exc) + "\n")
            sys.stderr.flush()
        return 0

    def disableModule(self, name):
        runFile = os.path.join(self.root, "etc", "init", name + ".auto")
        if not os.path.exists(runFile):
            print "%s is not enabled" % name
            return 0
        try:
            os.remove(runFile)
            print "disabled %s" % name
        except Exception, exc:
            sys.stderr.write(str(exc) + "\n")
            sys.stderr.flush()
        return 0

    def isModuleEnabled(self, name):
        runFile = os.path.join(self.root, "etc", "init", name + ".auto")
        return os.path.exists(runFile) == True

    # Return the module name from a path
    def moduleName(self, path):
        return os.path.splitext(os.path.basename(path))[0]

    # Returns a module's lockfile
    def lockFile(self, module):
        return os.path.join(self.root, "var", "run", module + ".pid")

    # Returns a module's runfile
    def runFile(self, module):
        return os.path.join(self.root, "var", "run", module + ".run")

    # Returns a module's logfile
    def logFile(self, module):
        return os.path.join(self.root, "var", "log", module + ".log")

    # Returns the binary file path of a given module name
    def binaryFile(self, module):
        # return os.path.join(self.root, "bin/" + module)
        return module

    def start(self, module, binary, params, nohup=False):
        cmd = binary + " " + params + " >" + self.logFile(module) + " 2>&1"
        if nohup:
            cmd = "nohup " + cmd + " &"
        return os.system(cmd)

    def stop(self, module, timeout):
        return self.killWait(module, timeout)

    def tryLock(self, module):
        return subprocess.call("trylock " + self.lockFile(module), shell=True) == 0

    def killWait(self, module, timeout):
        lockfile = self.lockFile(module)

        # Open pid file
        f = open(lockfile, "r")

        # Try to read the pid
        try:
            pid = int(f.readline())
        except:
            f.close()
            raise

        # Kill process with pid
        subprocess.call("kill %d" % pid, shell=True)
        if subprocess.call("waitlock %d \"%s\"" % (timeout, lockfile), shell=True) != 0:
            print "timeout exceeded"
            subprocess.call("kill -9 %d" % pid, shell=True)

        # Remove pid file
        try:
            os.remove(lockfile)
        except:
            pass

        return True

    def processTemplate(self, templateFile, paths, params, printError=False):
        self.last_template_file = None

        for tp in paths:
            if os.path.exists(os.path.join(tp, templateFile)):
                break

        else:
            if printError:
                print "Error: template %s not found" % (templateFile)
            return ""

        filename = os.path.join(tp, templateFile)
        self.last_template_file = filename

        try:
            t = Template(open(filename).read())
        except:
            if printError:
                print "Error: template %s not readable" % filename
            return ""

        params['date'] = time.ctime()
        params['template'] = filename

        while True:
            try:
                return t.substitute(params)

            except KeyError, e:
                print "warning: $%s is not defined in %s" % (
                    e.args[0], filename)
                params[e.args[0]] = ""

            except ValueError, e:
                raise ValueError, "%s: %s" % (filename, str(e))

    def logStatus(self, name, isRunning, shouldRun, isEnabled):
        if self._csv == False:
            sys.stdout.write("%-20s is " % name)
            if not isRunning:
                sys.stdout.write("not ")
            sys.stdout.write("running")
            if not isRunning and shouldRun:
                sys.stdout.write(" [WARNING]")
            sys.stdout.write("\n")
        else:
            sys.stdout.write("%s;%d;%d;%d\n" % (
                name, int(isRunning), int(shouldRun), int(isEnabled)))
        sys.stdout.flush()

    def log(self, line):
        sys.stdout.write(line + "\n")
        sys.stdout.flush()


# The module interface which implementes the basic default operations.
# Each script can define its own handlers to customize the behaviour.
# Available handlers:
#  start()
#  stop()
#  check()
#  status(shouldRun)
#  setup(params = dict{name, values as []})
#  updateConfig()
class Module:
    def __init__(self, env, name):
        self.env = env
        self.name = name
        # The start order
        self.order = 100
        # Defines if this is a kernel module or not.
        # Kernel modules are always started
        self.isKernelModule = False
        # Defines if this is a config only module
        self.isConfigModule = False
        # Set default timeout when stopping a module to 10 seconds before killing it
        self.killTimeout = 10

    def _get_start_params(self):
        # Run as daemon
        params = "-D"

        # Enable syslog if configured
        if self.env.syslog == True:
            params = params + "s"

        params = params + " -l " + self.env.lockFile(self.name)
        return params

    def _run(self):
        return self.env.start(self.name, self.env.binaryFile(self.name), self._get_start_params())

    def isRunning(self):
        return self.env.tryLock(self.name) == False

    def start(self):
        if self.isRunning():
            self.env.log("%s is already running" % self.name)
            return 1

        self.env.log("starting %s" % self.name)
        return self._run()

    def stop(self):
        if not self.isRunning():
            self.env.log("%s is not running" % self.name)
            return 1

        self.env.log("shutting down %s" % self.name)
        # Default timeout to 10 seconds
        return self.env.stop(self.name, self.killTimeout)

    # Check is the same as start. If a module should be checked
    # is decided by the control script which check the existence
    # of a corresponding run file.
    def check(self):
        return self.start()

    def status(self, shouldRun):
        self.env.logStatus(self.name, self.isRunning(), shouldRun, self.env.isModuleEnabled(
            self.name) or isinstance(self, CoreModule))

    def requiresKernelModules(self):
        # The default handler triggers a start of kernel modules before updating
        # its configuration
        return True

    def updateConfigProxy(self):
        # This function must return either a string containing the module name
        # of the proxy module that should be configured as well or None.
        return None

    def updateConfig(self):
        # This function must return a number indicating the error code where
        # 0 means no error. The default handler doesn't do anything.
        return 0

    def printCrontab(self):
        # The default handler doesn't do anything
        return 0

    def supportsAliases(self):
        # The default handler does not support aliases
        return False


# Define a kernel core module which is started always
class CoreModule(Module):
    def __init__(self, env, name):
        Module.__init__(self, env, name)
