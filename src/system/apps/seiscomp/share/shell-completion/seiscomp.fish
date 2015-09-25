set -l seiscompCommands install-deps setup shell enable disable start stop restart check status list exec update-config alias print help

function __fish_seiscomp_needs_command
  set cmd (commandline -opc)
  if [ (count $cmd) -eq 1 -a $cmd[1] = 'seiscomp' ]
    return 0
  end
  return 1
end

function __fish_seiscomp_using_command
  set cmd (commandline -opc)
  if [ (count $cmd) -gt 1 ]
    if [ $argv[1] = $cmd[2] ]
      echo "using:"
      return 0
    end
  end
  return 1
end

function __fish_seiscomp_modules
  command seiscomp list modules | cut -d " " -f1
end

function __fish_seiscomp_command_help
    command seiscomp help $argv | head -1
end

function __fish_seiscomp_binaries
    command ls (seiscomp exec env|grep SEISCOMP_ROOT|cut -d "=" -f2)/bin
end

set -l seiscompBinaries (__fish_seiscomp_binaries)

complete -f -c seiscomp -n "__fish_seiscomp_needs_command" -a "install-deps" -d "Installs OS dependencies to run SeisComP3"
complete -f -c seiscomp -n "__fish_seiscomp_needs_command" -a "setup" -d "Initializes the configuration of all available modules"
complete -f -c seiscomp -n "__fish_seiscomp_needs_command" -a "shell" -d "Launches the SeisComP shell"
complete -f -c seiscomp -n "__fish_seiscomp_needs_command" -a "enable" -d "Enables all given modules to be started"
complete -f -c seiscomp -n "__fish_seiscomp_needs_command" -a "disable" -d "Disables all given modules"
complete -f -c seiscomp -n "__fish_seiscomp_needs_command" -a "start" -d "Starts all enabled modules or a list of modules given"
complete -f -c seiscomp -n "__fish_seiscomp_needs_command" -a "stop" -d  "Stops all enabled modules or a list of modules given"
complete -f -c seiscomp -n "__fish_seiscomp_needs_command" -a "restart" -d "Restarts all enabled modules or a list of modules given"
complete -f -c seiscomp -n "__fish_seiscomp_needs_command" -a "check" -d "Checks if a started module is still running"
complete -f -c seiscomp -n "__fish_seiscomp_needs_command" -a "list" -d "Prints the status of all or a list of modules"
complete -f -c seiscomp -n "__fish_seiscomp_needs_command" -a "exec" -d "Executes a command like calling a command from commandline"
complete -f -c seiscomp -n "__fish_seiscomp_needs_command" -a "update-config" -d "Updates the configuration of all available modules"
complete -f -c seiscomp -n "__fish_seiscomp_needs_command" -a "alias" -d "Creates/removes symlinks to applications"
complete -f -c seiscomp -n "__fish_seiscomp_needs_command" -a "print" -d "prints crontab entries of all registered or given modules"
complete -f -c seiscomp -n "__fish_seiscomp_needs_command" -a "help" -d "help"

# help
complete -f -c seiscomp -n "__fish_seiscomp_using_command help" -a "$seiscompCommands"

# exec
complete -f -c seiscomp -n "__fish_seiscomp_using_command exec" -a "$seiscompBinaries"

# print
complete -f -c seiscomp -n "__fish_seiscomp_using_command print" -a "env crontab"

# alias
complete -f -c seiscomp -n "__fish_seiscomp_using_command alias" -a "create remove"

# list
complete -f -c seiscomp -n "__fish_seiscomp_using_command list" -a "modules aliases enabled disabled"

# complete modules
for mcmd in enable disable start stop restart check status update-config
    complete -f -c seiscomp -n "__fish_seiscomp_using_command $mcmd" -a "(__fish_seiscomp_modules)"
end
