import seiscomp3.Kernel, sys

class Module(seiscomp3.Kernel.Module):
  def __init__(self, env):
    seiscomp3.Kernel.Module.__init__(self, env, env.moduleName(__file__))

  def updateConfigProxy(self):
    return "trunk"

  def updateConfig(self):
    # By default the "trunk" module must be configured to write the
    # bindings into the database
    return 0

  def supportsAliases(self):
    # The default handler does not support aliases
    return True
