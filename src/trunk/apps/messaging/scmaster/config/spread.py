import os
import time
import seiscomp3.Kernel


# The kernel module which starts spread if enabled
class Module(seiscomp3.Kernel.CoreModule):
    def __init__(self, env):
        seiscomp3.Kernel.CoreModule.__init__(
            self, env, env.moduleName(__file__))
        # Highest priority
        self.order = -2

        # Default values
        self.messaging = True
        self.messagingPort = 4803

        self.suppressOutput = False

        try:
            self.messaging = self.env.getBool("messaging.enable")
        except:
            pass
        try:
            self.messagingPort = self.env.getInt("messaging.port")
        except:
            pass

    def _run(self):
        self.suppressOutput = True
        # Update configuration
        self.updateConfig()
        self.suppressOutput = False

        params = self.env.lockFile(self.name) + " spread -n localhost"
        spread_conf = os.path.join(
            self.env.root, "var", "lib", "spread", "spread.conf")

        params = params + " -c \"" + spread_conf + "\""
        res = self.env.start(self.name, "run_with_lock", params, nohup=True)
        time.sleep(2)
        return res

    def start(self):
        if not self.messaging:
            print "[kernel] %s is disabled by config" % self.name
            return 0

        return seiscomp3.Kernel.CoreModule.start(self)

    def stop(self):
        return seiscomp3.Kernel.CoreModule.stop(self)

    def check(self):
        if not self.messaging:
            print "[kernel] %s is disabled by config" % self.name
            return 0

        return seiscomp3.Kernel.CoreModule.check(self)

    def updateConfig(self):
        spread_conf_dir = os.path.join(self.env.root, "var", "lib", "spread")

        # Create config directory in var/lib/spread
        if not os.path.exists(spread_conf_dir):
            try:
                os.makedirs(spread_conf_dir)
            except Exception, e:
                if not self.suppressOutput:
                    print "%s: %s" % (spread_conf_dir, str(e))
                return 1

        spread_conf = os.path.join(spread_conf_dir, "spread.conf")

        tp = [os.path.join(os.environ['HOME'], ".seiscomp3", "templates"),
              os.path.join(self.env.SEISCOMP_ROOT, "share", "templates")]
        params = {'spread.port': self.messagingPort}

        content = self.env.processTemplate("spread.conf.tpl", tp, params, True)
        if content:
            if not self.suppressOutput:
                print "using configuration template in %s" % self.env.last_template_file
            try:
                cfg = open(spread_conf, 'w')
            except:
                if not self.suppressOutput:
                    print "%s: failed to write: abort" % os.path.abspath(
                        spread_conf)
                return 1

            cfg.write(self.env.processTemplate(
                "spread.conf.tpl", tp, params, True))
            cfg.close()
        else:
            if not self.suppressOutput:
                print "WARNING: no configuration template found -> empty configuration"
            try:
                os.remove(spread_conf)
            except:
                pass

        return 0

    def status(self, shouldRun):
        if not self.messaging:
            shouldRun = False
        seiscomp3.Kernel.CoreModule.status(self, shouldRun)
