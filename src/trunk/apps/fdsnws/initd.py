import os
import subprocess
import time

import seiscomp3.Kernel


class Module(seiscomp3.Kernel.Module):
    def __init__(self, env):
        seiscomp3.Kernel.Module.__init__(self, env, env.moduleName(__file__))

    def supportsAliases(self):
        # The default handler does not support aliases
        return True

    def reload(self):
        if not self.isRunning():
            self.env.log('{0} is not running'.format(self.name))
            return 1

        self.env.log('reloading {0}'.format(self.name))

        lockfile = self.env.lockFile(self.name)
        reloadfile = os.path.join(os.path.dirname(lockfile),
                                  '{0}.reload'.format(self.name))

        # Open pid file
        with open(lockfile, "r") as f:
            # Try to read the pid
            pid = int(f.readline())

            # touch reload file
            open(reloadfile, 'a').close()

            if not os.path.isfile(reloadfile):
                self.env.log('could not touch reload file: {0}' \
                             .format(reloadfile))
                return 1

            # Send SIGHUP
            subprocess.call("kill -s HUP %d" % pid, shell=True)

            # wait for reload file to disappear
            for _ in range(0, int(self.reloadTimeout * 5)):
                time.sleep(0.2)
                if not os.path.isfile(reloadfile):
                    return 0

            self.env.log('timeout exceeded')

        return 1

# Uncomment for authbind (running service on privileged ports)
#  def _run(self):
#    params = "--depth 2 " + self.env.binaryFile(self.name) + " " + self._get_start_params()
#    binaryPath = "authbind"
#    return self.env.start(self.name, binaryPath, params)
