import os, sys
import seiscomp3.Kernel


class Module(seiscomp3.Kernel.Module):
  def __init__(self, env):
    seiscomp3.Kernel.Module.__init__(self, env, "syncarc")
    # This is a config module that will execute sync_arc if requested
    # to merge inventory from remote nodes using arclink
    self.isConfigModule = True
    self.order = 1000

  def updateConfig(self):
    messaging = True
    syncOnUpdate = False
    result = 0

    try: messaging = self.env.getBool("messaging.enable")
    except: pass
    try: syncOnUpdate = self.env.getBool("inventory.syncOnUpdate")
    except: pass

    # If messaging is disabled in kernel.cfg, do not do anything
    if not messaging:
      sys.stdout.write("- messaging disabled, nothing to do\n")
      return 0

    # Merge inventory
    if syncOnUpdate:
        result = os.system("sync_arc --merge")

    return result
