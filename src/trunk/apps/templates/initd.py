import seiscomp3.Kernel

class Module(seiscomp3.Kernel.Module):
  def __init__(self, env):
    seiscomp3.Kernel.Module.__init__(self, env, env.moduleName(__file__))


  def supportsAliases(self):
    # The default handler does not support aliases
    return True
