import seiscomp3.Kernel


class Module(seiscomp3.Kernel.Module):
    def __init__(self, env):
        seiscomp3.Kernel.Module.__init__(self, env, env.moduleName(__file__))

    def supportsAliases(self):
        # The default handler does not support aliases
        return True

# Uncomment for authbind (running service on privileged ports)
#  def _run(self):
#    params = "--depth 2 " + self.env.binaryFile(self.name) + " " + self._get_start_params()
#    binaryPath = "authbind"
#    return self.env.start(self.name, binaryPath, params)
