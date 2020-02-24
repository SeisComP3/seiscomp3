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
import glob

try:
    import readline
except:
    pass

import seiscomp3.Config


def split_tokens(line):
    return line.split()


def convert_wildcard(s):
    wild = s.split(".")
    if len(wild) > 2:
        raise Exception("station selector: only one dot allowed")

    # Add station wildcard if only network is given
    if len(wild) == 1:
        wild.append('*')
    for i in range(len(wild)):
        if len(wild[i]) == 0:
            wild[i] = '*'
    return '_'.join(wild)


def convert_stations(s):
    toks = s.split(".")
    if len(toks) != 2:
        raise Exception("station: expected format: NET.STA")
    return '_'.join(toks)


class CLI:
    """
    Simple console shell.
    """

    def run(self, env):
        self.env = env

        sys.stdout.write('''\
%s
SeisComP shell
%s

Welcome to the SeisComP interactive shell. You can get help about
available commands with 'help'. 'exit' leaves the shell.

''' % (("="*80), ("="*80)))

        prompt = "$ "
        while True:
            try:
                line = raw_input(prompt).strip()
            except NameError:
                line = input(prompt).strip()
            toks = split_tokens(line)
            if len(toks) == 0:
                continue

            if line == "exit" or line == "quit":
                break

            self.handleCommand(toks[0], toks[1:])

    def handleCommand(self, cmd, args):
        try:
            if cmd == "help":
                return self.commandHelp(args)
            elif cmd == "list":
                return self.commandList(args)
            elif cmd == "delete":
                return self.commandDelete(args)
            elif cmd == "print":
                return self.commandPrint(args)
            elif cmd == "set":
                return self.commandSet(args)
            elif cmd == "remove":
                return self.commandRemove(args)

            raise Exception("Unknown command: %s" % cmd)
        except Exception as e:
            sys.stdout.write("%s\n" % str(e))
            return False

    def commandHelp(self, args):
        if len(args) == 0:
            sys.stdout.write("""\
Commands:
  list stations
    Lists all available stations keys.

  list profiles {mod}
    Lists all available profiles of a module.

  list modules {sta}
    Lists all bound modules of a station incl. profiles (if used).

  add profile {mod} {profile}
    Adds a new profile for given module. If the profile exists already an error
    is raised.

  add binding {mod} {sta}
    Adds a new binding for given module and station. If the binding already
    exists an error is raised.

  delete profile {mod} {profile}
    Deletes the given profile of given module. If the profile does not exist an
    error is raised.
    The module is removed from all stations that are using this profile.

  delete binding {mod} {sta}
    Deletes the binding for given module and station. If the station is bound
    to module mod using a profile the binding is kept, removed otherwise.
    An existing binding file (etc/key/[mod]/station_[sta]) is deleted in any
    case.

  edit profile {mod} {profile}
    Starts the interactive configuration wizard for the profile if a description
    exists. Otherwise an error is raised. Changes to the profile have effect
    on all stations using this profile.

  edit binding {mod} {sta}
    Starts the interactive configuration wizard for the binding if a description
    exists. Otherwise an error is raised.

  edit module {mod}
    Starts the interactive configuration wizard for the given module if a
    description exists. Otherwise an error is raised.

  print station {sta}
    Dumps all set binding parameters for the given station.

  set profile {mod} {profile} {sta-sel}
    Sets for all selected stations a binding profile of a module.
    The resulting station file looks like this:
    ...
    mod:profile
    ...

    This command checks for the existence of the specified profile

  set module {mod} {sta-sel}
    Binds all selected stations to given module. No profiles are used
    and if any of the stations is already using a profile it is removed.
    The resulting station key file looks like this:
    ...
    mod
    ...

  remove profile {mod} {profile} {sta-sel}
    Removes the binding profile of given module for all selected stations if
    module is bound already to that station.
    As a result all selected stations that are bound to the given module already
    will use a station key file afterwards.

    mod:profile  ->   mod

  remove module {mod} {sta-sel}
    Unbinds given module from selected stations. The line that refers to the
    given module is completely removed from the station key files.

  exit
    Exit the shell.

  quit
    Alias for exit.
""")

    def commandList(self, args):
        if len(args) == 0:
            raise Exception("Missing operand")

        if args[0] == "stations":
            if len(args) > 2:
                raise Exception("Too many arguments")

            if len(args) > 1:
                wild = convert_wildcard(args[1])
            else:
                wild = "*"

            stas = []
            for f in sorted(glob.glob(os.path.join(self.env.key_dir, "station_" + wild))):
                stas.append(os.path.basename(f)[8:].replace("_", "."))

            for s in stas:
                print(s)

            return True

        elif args[0] == "profiles":
            if len(args) > 2:
                raise Exception("Too many arguments")
            if len(args) < 2:
                raise Exception("Expected: mod")

            module = args[1]

            profs = []
            for f in sorted(glob.glob(os.path.join(self.env.key_dir, module, "profile_*"))):
                print(os.path.basename(f)[8:])

            return True

        elif args[0] == "modules":
            if len(args) > 2:
                raise Exception("Too many arguments")
            if len(args) < 2:
                raise Exception("Expected: sta")

            sta = convert_stations(args[1])

            f = os.path.join(self.env.key_dir, "station_" + sta)
            if not os.path.exists(f):
                raise Exception("%s: station key does not exists" % args[1])

            for l in [line.strip() for line in open(f, "r").readlines()]:
                if l.startswith("#"):
                    continue
                if len(l) == 0:
                    continue
                print(l)

            return True

        else:
            raise Exception("Invalid argument: %s" % args[0])

    def commandDelete(self, args):
        if len(args) == 0:
            raise Exception("Missing operand")

        if args[0] == "profile":
            if len(args) > 3:
                raise Exception("Too many arguments")
            if len(args) < 3:
                raise Exception("Expected: mod profile")

            module = args[1]
            profile = args[2]

            if not os.path.exists(os.path.join(self.env.key_dir, module, "profile_" + profile)):
                raise Exception("%s/%s: profile not found" % (module, profile))

            os.remove(os.path.join(self.env.key_dir,
                                   module, "profile_" + profile))

            modified = 0
            for f in glob.glob(os.path.join(self.env.key_dir, "station_*")):
                lines = [line.strip() for line in open(f, "r").readlines()]

                new_lines = []
                is_modified = False

                for i in range(len(lines)):
                    line = lines[i]

                    # Comment line
                    if line.startswith("#"):
                        new_lines.append(line)
                        continue

                    # Empty line
                    if len(line) == 0:
                        new_lines.append(line)
                        continue

                    toks = line.split(':')

                    # Wrong module name
                    if toks[0] != module:
                        new_lines.append(line)
                        continue

                    # Profile found
                    if len(toks) > 1 and toks[1] == profile:
                        # Filter line
                        is_modified = True
                        continue

                    new_lines.append(line)

                if is_modified:
                    modified += 1
                    try:
                        open(f, "w").write('\n'.join(new_lines))
                    except Exception as e:
                        sys.stdout.write("%s: %s\n" % (f, str(e)))

            sys.stdout.write("OK, %d files modified\n" % modified)

            return True

        elif args[0] == "binding":
            if len(args) > 3:
                raise Exception("Too many arguments")
            if len(args) < 3:
                raise Exception("Expected: mod profile")

            module = args[1]
            sta = convert_stations(args[2])

            if not os.path.exists(os.path.join(self.env.key_dir, module, "station_" + sta)):
                raise Exception("%s/%s: binding not found" % (module, args[2]))

            os.remove(os.path.join(self.env.key_dir, module, "station_" + sta))

            f = os.path.join(self.env.key_dir, "station_" + sta)
            try:
                lines = [line.strip() for line in open(f, "r").readlines()]
            except:
                pass

            new_lines = []
            is_modified = False

            for i in range(len(lines)):
                line = lines[i]

                # Comment line
                if line.startswith("#"):
                    new_lines.append(line)
                    continue

                # Empty line
                if len(line) == 0:
                    new_lines.append(line)
                    continue

                toks = line.split(':')

                # Wrong module name
                if toks[0] != module:
                    new_lines.append(line)
                    continue

                # Profile found
                if len(toks) == 1:
                    # Filter line
                    is_modified = True
                    continue

                new_lines.append(line)

            if is_modified:
                try:
                    open(f, "w").write('\n'.join(new_lines))
                except Exception as e:
                    sys.stdout.write("%s: %s\n" % (f, str(e)))

            return True

        else:
            raise Exception("Invalid argument: %s" % args[0])

    def commandPrint(self, args):
        if len(args) == 0:
            raise Exception("Missing operand")

        if args[0] == "station":
            if len(args) != 2:
                raise Exception("missing argument, expected: sta")

            sta = convert_stations(args[1])
            key = os.path.join(self.env.key_dir, "station_" + sta)
            try:
                lines = [line.strip() for line in open(key, "r").readlines()]
            except IOError as e:
                raise Exception("%s: station not configured" % sta)
            except Exception as e:
                raise Exception("%s: unexpected error: %s" % (sta, str(e)))

            first = True

            for line in lines:
                # Comment line
                if line.startswith("#"):
                    continue
                # Empty line
                if len(line) == 0:
                    continue

                toks = line.split(':')

                if len(toks) == 1:
                    binding = os.path.join(
                        self.env.key_dir, toks[0], "station_" + sta)
                else:
                    binding = os.path.join(
                        self.env.key_dir, toks[0], "profile_" + toks[1])

                if not first:
                    sys.stdout.write("\n")

                first = False
                sys.stdout.write("[%s]\n" % toks[0])
                sys.stdout.write("%s\n" % binding)
                try:
                    data = open(binding).read()
                    sys.stdout.write("-"*80 + "\n")
                    sys.stdout.write(data)
                    sys.stdout.write("-"*80 + "\n")
                except IOError as e:
                    sys.stdout.write("!binding not found\n")
                except Exception as e:
                    sys.stdout.write("!unexpected error: %s\n" % str(e))

        else:
            raise Exception("Invalid argument: %s" % args[0])

    def commandSet(self, args):
        if len(args) == 0:
            raise Exception("Missing operand")

        if args[0] == "profile":
            if len(args) != 4:
                raise Exception(
                    "missing arguments, expected: module profile station-selector")

            module = args[1]
            profile = args[2]

            wild = convert_wildcard(args[3])

            if not os.path.exists(os.path.join(self.env.key_dir, module, "profile_" + profile)):
                raise Exception("%s/%s: profile not found" % (module, profile))

            modified = 0
            for f in glob.glob(os.path.join(self.env.key_dir, "station_" + wild)):
                lines = [line.strip() for line in open(f, "r").readlines()]

                module_found = False
                is_modified = False

                for i in range(len(lines)):
                    line = lines[i]

                    # Comment line
                    if line.startswith("#"):
                        continue
                    # Empty line
                    if len(line) == 0:
                        continue

                    toks = line.split(':')

                    # Wrong module name
                    if toks[0] != module:
                        continue

                    module_found = True

                    # No profile
                    if len(toks) == 1:
                        toks.append("")
                    # Profile already set
                    elif toks[1] == profile:
                        continue

                    toks[1] = profile
                    lines[i] = ':'.join(toks)

                    is_modified = True

                if not module_found:
                    lines.append("%s:%s\n" % (module, profile))
                    is_modified = True

                if is_modified:
                    modified += 1
                    try:
                        open(f, "w").write('\n'.join(lines))
                    except Exception as e:
                        sys.stdout.write("%s: %s\n" % (f, str(e)))

            sys.stdout.write("OK, %d files modified\n" % modified)

            return True

        elif args[0] == "module":
            if len(args) != 3:
                raise Exception(
                    "missing arguments, expected: module station-selector")

            module = args[1]

            wild = convert_wildcard(args[2])

            modified = 0
            for f in glob.glob(os.path.join(self.env.key_dir, "station_" + wild)):
                lines = [line.strip() for line in open(f, "r").readlines()]

                module_found = False
                is_modified = False

                for i in range(len(lines)):
                    line = lines[i]

                    # Comment line
                    if line.startswith("#"):
                        continue
                    # Empty line
                    if len(line) == 0:
                        continue

                    toks = line.split(':')

                    # Wrong module name
                    if toks[0] != module:
                        continue

                    module_found = True

                    lines[i] = module

                    is_modified = True

                if not module_found:
                    lines.append("%s\n" % module)
                    is_modified = True

                if is_modified:
                    modified += 1
                    try:
                        open(f, "w").write('\n'.join(lines))
                    except Exception as e:
                        sys.stdout.write("%s: %s\n" % (f, str(e)))

            sys.stdout.write("OK, %d files modified\n" % modified)

            return True

        else:
            raise Exception("Invalid argument: %s" % args[0])

    def commandRemove(self, args):
        if len(args) == 0:
            raise Exception("Missing operand")

        if args[0] == "profile":
            if len(args) != 4:
                raise Exception(
                    "Missing arguments, expected: module profile station-selector")

            module = args[1]
            profile = args[2]

            wild = convert_wildcard(args[3])

            modified = 0
            for f in glob.glob(os.path.join(self.env.key_dir, "station_" + wild)):
                lines = [line.strip() for line in open(f, "r").readlines()]

                is_modified = False
                for i in range(len(lines)):
                    line = lines[i]

                    # Comment line
                    if line.startswith("#"):
                        continue

                    # Empty line
                    if len(line) == 0:
                        continue

                    toks = line.split(':')

                    # No profile
                    if len(toks) == 1:
                        continue

                    # Wrong module name
                    if toks[0] != module:
                        continue

                    # Wrong profile name
                    if toks[1] != profile:
                        continue

                    lines[i] = module
                    is_modified = True

                    continue

                if is_modified:
                    modified += 1

                    if (len(lines) > 0) and (len(lines[-1]) > 0):
                        lines.append("")

                    try:
                        open(f, "w").write('\n'.join(lines))
                    except Exception as e:
                        sys.stdout.write("%s: %s\n" % (f, str(e)))

            sys.stdout.write("OK, %d files modified\n" % modified)

            return True

        elif args[0] == "module":
            if len(args) != 3:
                raise Exception(
                    "Missing arguments, expected: module station-selector")

            module = args[1]

            wild = convert_wildcard(args[2])

            modified = 0
            for f in glob.glob(os.path.join(self.env.key_dir, "station_" + wild)):
                lines = [line.strip() for line in open(f, "r").readlines()]
                new_lines = []

                is_modified = False
                for i in range(len(lines)):
                    line = lines[i]

                    # Comment line
                    if line.startswith("#"):
                        new_lines.append(line)
                        continue

                    # Empty line
                    if len(line) == 0:
                        new_lines.append(line)
                        continue

                    toks = line.split(':')

                    # Wrong module name
                    if toks[0] != module:
                        new_lines.append(line)
                        continue

                    # Filter line
                    is_modified = True

                if is_modified:
                    modified += 1
                    if (len(new_lines) > 0) and (len(new_lines[-1]) > 0):
                        new_lines.append("")

                    try:
                        open(f, "w").write('\n'.join(new_lines))
                    except Exception as e:
                        sys.stdout.write("%s: %s\n" % (f, str(e)))

                    try:
                        os.remove(os.path.join(self.env.key_dir,
                                               module, os.path.basename(f)))
                    except:
                        pass

            sys.stdout.write("OK, %d files modified\n" % modified)

            return True

        else:
            raise Exception("Invalid argument: %s" % args[0])
