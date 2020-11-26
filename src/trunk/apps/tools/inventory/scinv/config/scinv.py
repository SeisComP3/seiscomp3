import os
import sys
import seiscomp3.Kernel


class Module(seiscomp3.Kernel.Module):
    def __init__(self, env):
        seiscomp3.Kernel.Module.__init__(self, env, "inventory")
        # This is a config module which synchronizes bindings with the database
        self.isConfigModule = True
        # Give this module a high priority to be executed at first (unless
        # another module defines a negative value. It ensures that successive
        # modules can read an up-to-date inventory and use the latest rc files.
        self.order = 0

    def updateConfig(self):
        messaging = True
        messagingPort = 4803

        try:
            messaging = self.env.getBool("messaging.enable")
        except:
            pass
        try:
            messagingPort = self.env.getInt("messaging.port")
        except:
            pass

        # If messaging is disabled in kernel.cfg, do not do anything
        if not messaging:
            sys.stdout.write("- messaging disabled, nothing to do\n")
            return 0

        # Synchronize inventory
        return os.system("scinv sync --console=1 -H localhost:%d --filebase \"%s\" --rc-dir \"%s\" --key-dir \"%s\""
                         % (messagingPort,
                            os.path.join(self.env.root, "etc", "inventory"),
                            os.path.join(self.env.root, "var", "lib", "rc"),
                            os.path.join(self.env.root, "etc", "key")))
