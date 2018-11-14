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
import getpass

try:
    # Python 2.5
    from xml.etree import ElementTree
    from xml.etree.ElementTree import Element
    from xml.parsers.expat import ExpatError as ParseError
except ImportError:
    from elementtree import ElementTree
    from elementtree.ElementTree import Element
    from xml.parsers.expat import ExpatError as ParseError

try:
    import readline
except:
    pass


import seiscomp3.Config


def tagname(element):
    names = element.tag.split("}")
    if len(names) == 0:
        return ""

    return names.pop()


def oneliner(txt):
    return txt.strip().replace("\n", "")


def block(txt, width=80):
    lines = [l.strip() for l in txt.strip().replace("\r", "").split('\n')]
    line = "\n".join(lines)

    current = 0
    lines = []

    while current < len(line):
        end = line.find('\n', current)
        if (end == -1) or (end - current > width):
            if len(line) - current > width:
                end = line.rfind(' ', current, current+width)
                if end == -1:
                    end = line.find(' ', current)
                    if end == -1:
                        end = len(line)
            else:
                end = len(line)

        lines.append(line[current:end].strip())

        current = end + 1

    return lines


class SetupNode:
    def __init__(self, parent, left, right, inp):
        self.parent = parent
        self.prev = left
        self.next = right
        self.input = inp
        self.child = None
        self.activeChild = None

        self.modname = ""
        self.groupname = ""
        self.path = ""
        self.value = ""
        self.lastInGroup = False


class Option:
    """
    Setup input option wrapper.
    """

    def __init__(self, value):
        self.value = value
        self.desc = None
        self.inputs = []


class Input:
    """
    Setup input wrapper.
    """

    def __init__(self, name, type, default_value=None):
        self.name = name
        self.type = type
        self.default_value = default_value
        self.text = None
        self.desc = None
        self.echo = None
        self.options = []


def dumpTree(cfg, node):
    if node.input:
        cfg.setString(node.modname + "." + node.path, node.value)

    if node.activeChild:
        dumpTree(cfg, node.activeChild)
    if not node.lastInGroup and not node.next is None:
        dumpTree(cfg, node.next)


class Simple:
    """
    Simple console setup handler that parses all description xml files
    and extracts the setup part. It asks for all available setting line
    by line and passes the resulting configuration back which is then
    passed to all init modules that have a setup method.
    """

    def run(self, env):
        desc_pattern = os.path.join(
            env.SEISCOMP_ROOT, "etc", "descriptions", "*.xml")
        xmls = glob.glob(desc_pattern)

        setup_groups = {}

        for f in xmls:
            try:
                tree = ElementTree.parse(f)
            except ParseError, (err):
                sys.stderr.write("%s: parsing XML failed: %s\n" % (f, err))
                continue

            root = tree.getroot()
            if tagname(root) != "seiscomp":
                sys.stderr.write(
                    "%s: wrong root tag, expected 'seiscomp'\n" % f)
                continue

            # Read all modules
            mods = tree.findall("module")

            for mod in mods:
                modname = mod.get('name')
                if not modname:
                    sys.stderr.write("%s: skipping module without name\n" % f)
                    continue

                if setup_groups.has_key(modname):
                    raise Exception(
                        "%s: duplicate module name: %s" % (f, modname))

                setup = mod.find("setup")
                if setup is None:
                    continue

                groups = setup.findall("group")
                if len(groups) == 0:
                    continue

                setup_groups[modname] = groups

            # Read all plugin's
            plugins = tree.findall("plugin")

            for plugin in plugins:
                try:
                    modname = plugin.find('extends').text.strip()
                except:
                    raise Exception("%s: plugin does not define 'extends'" % f)

                if modname.find('\n') >= 0:
                    raise Exception(
                        "%s: wrong module name in plugin.extends: no newlines allowed" % f)

                if not modname:
                    sys.stderr.write("%s: skipping module without name\n" % f)
                    continue

                setup = plugin.find("setup")
                if setup is None:
                    continue

                groups = setup.findall("group")
                if len(groups) == 0:
                    continue

                try:
                    setup_groups[modname] += groups
                except:
                    setup_groups[modname] = groups

        self.setupTree = SetupNode(None, None, None, None)
        self.paths = []

        for name, groups in sorted(setup_groups.items()):
            self.addGroups(self.setupTree, name, groups)

        # Always descend to the first child (if available)
        self.setupTree.activeChild = self.setupTree.child
        self.currentNode = self.setupTree.activeChild

        sys.stdout.write(
            '''\
====================================================================
SeisComP setup
====================================================================

This initializes the configuration of your installation.
If you already made adjustments to the configuration files
be warned that this setup will overwrite existing parameters
with default values. This is not a configurator for all
options of your setup but helps to setup initial standard values.

--------------------------------------------------------------------
Hint: Entered values starting with a dot (.) are handled
      as commands. Available commands are:

      quit: Quit setup without modification to your configuration.
      back: Go back to the previous parameter.
      help: Show help about the current parameter (if available).

      If you need to enter a value with a leading dot, escape it
      with backslash, e.g. "\.value".
--------------------------------------------------------------------

''')

        try:
            self.fillTree()
        except StopIteration, e:
            raise Exception("aborted by user")

        cfg = seiscomp3.Config.Config()
        dumpTree(cfg, self.setupTree)

        return cfg

    def addGroups(self, node, modname, groups):
        for g in groups:
            self.addInputs(None, node, modname, g.get(
                'name'), g, g.get('name', "") + ".")

    def addInputs(self, obj, parent, modname, group, xml, prefix):
        last = parent.child

        # find the last child and add the current list to it
        while not last is None:
            if last.next is None:
                break
            last = last.next

        inputs = xml.findall("input")
        for inp in inputs:
            name = inp.get('name')
            if not name:
                raise Exception("%s: no name defined" % prefix)

            input_ = Input(name, inp.get('type'), inp.get('default'))
            try:
                input_.text = oneliner(inp.find('text').text)
            except:
                input_.text = input_.name

            try:
                input_.desc = block(inp.find('description').text)
            except:
                pass

            input_.echo = inp.get('echo')

            if obj:
                obj.inputs.append(input_)

            node = SetupNode(parent, last, None, input_)
            node.path = prefix + input_.name
            node.value = input_.default_value
            node.modname = modname
            node.group = group

            if not last is None:
                last.next = node
            last = node

            if parent.child is None:
                parent.child = last

            opts = inp.findall("option")
            for opt in opts:
                value = opt.get('value')
                if not value:
                    raise Exception("%s: option without value" % prefix)

                option = Option(value)
                try:
                    option.desc = block(opt.find('description').text, 74)
                except:
                    pass
                input_.options.append(option)
                self.addInputs(option, node, modname,
                               group, opt, node.path + ".")

        if not obj is None and not last is None:
            last.lastInGroup = True

    def fillTree(self):
        while True:
            if not self.currentNode:
                sys.stdout.write("\nFinished setup\n--------------\n\n")
                sys.stdout.write("P) Proceed to apply configuration\n")
                sys.stdout.write("B) Back to last parameter\n")
                sys.stdout.write("Q) Quit without changes\n")

                value = raw_input('Command? [P]: ').upper()
                if value == "Q":
                    raise StopIteration()
                elif value == "P" or not value:
                    sys.stdout.write("\nRunning setup\n-------------\n\n")
                    return
                elif value == "B":
                    self.prevStep()
                    continue
                else:
                    sys.stdout.write("\nEnter either p, b or q\n")
                    continue

            if not self.currentNode.input:
                self.nextStep()
                continue

            default_value = self.valueToString(self.currentNode)

            isChoice = False
            isPassword = False
            if self.currentNode.input.echo == "password":
                isPassword = True

            node_text = default_value
            prompt = self.currentNode.input.text

            if isPassword:
                node_text = '*' * len(node_text)
                prompt += " (input not echoed)"

            if (not self.currentNode.input.type or self.currentNode.input.type != "boolean") and len(self.currentNode.input.options) > 0:
                idx = 0
                def_idx = 0
                for opt in self.currentNode.input.options:
                    sys.stdout.write("%2d) %s\n" % (idx, opt.value))
                    for l in opt.desc:
                        sys.stdout.write("      %s\n" % l)
                    if default_value == opt.value:
                        def_idx = idx
                    idx += 1
                isChoice = True
                prompt += " [%d]: " % def_idx
            else:
                prompt += " [%s]: " % node_text

            if self.currentNode.input.echo == "password":
                value = getpass.getpass(prompt)
            else:
                value = raw_input(prompt)

            if not value:
                value = default_value
            elif value == ".help":
                if self.currentNode.input.desc:
                    sys.stdout.write("\n%s\n\n" %
                                     "\n".join(self.currentNode.input.desc))
                else:
                    sys.stdout.write("\nSorry, no help available.\n\n")
                continue
            elif value == ".back":
                self.prevStep()
                continue
            elif value == ".quit":
                raise StopIteration()
            elif value.startswith("."):
                sys.stdout.write("Unknown command. Values starting with '.' are handled has commands such as\n"
                                 "'.help', '.quit' or '.back'. To use a leading dot in a value, escape it with '\'\n"
                                 "e.g. '\.color'\n")
                continue
            else:
                # Replace leading \. with .
                if value.startswith('\\.'):
                    value = value[1:]

                if isChoice:
                    try:
                        idx = int(value)
                    except:
                        idx = -1
                    if idx < 0 or idx >= len(self.currentNode.input.options):
                        sys.stdout.write("\nEnter a number between 0 and %d\n\n" % (
                            len(self.currentNode.input.options)-1))
                        continue
                    value = self.currentNode.input.options[idx].value

            if self.currentNode.input.type and self.currentNode.input.type == "boolean":
                if not value in ["yes", "no"]:
                    sys.stdout.write("Please enter 'yes' or 'no'\n")
                    continue

                if value == "yes":
                    value = "true"
                else:
                    value = "false"

            self.currentNode.value = value
            self.nextStep()

    def valueToString(self, node):
        if not node.input.type:
            if node.value is None:
                return ""
            return node.value

        if node.input.type == "boolean":
            if node.value == "true":
                return "yes"
            elif node.value == "false":
                return "no"
            else:
                return "yes"

        if node.value is None:
            return ""
        return node.value

    def prevStep(self):
        if len(self.paths) == 0:
            sys.stdout.write("No previous step available\n")
            return

        self.currentNode = self.paths.pop()

    def nextStep(self):
        self.currentNode.activeChild = None
        step = True

        # Choice?
        if len(self.currentNode.input.options) > 0:
            for opt in self.currentNode.input.options:
                if opt.value == self.currentNode.value:
                    if len(opt.inputs) > 0:
                        # Found new path to descend
                        child = self.currentNode.child

                        # Search corresponding child in the three
                        while child:
                            if child.input == opt.inputs[0]:
                                self.paths.append(self.currentNode)
                                self.currentNode.activeChild = child
                                self.currentNode = self.currentNode.activeChild
                                step = False
                                break

                            child = child.next

                    break

        if step:
            pushed = False
            if self.currentNode.next is None:
                parent = self.currentNode.parent

                while not parent is None:
                    if not parent.next is None:
                        self.paths.append(self.currentNode)
                        pushed = True
                        self.currentNode = parent
                        break

                    parent = parent.parent

                if parent is None:
                    self.paths.append(self.currentNode)
                    self.currentNode = None
                    return

            if not pushed:
                self.paths.append(self.currentNode)
            self.currentNode = self.currentNode.next
