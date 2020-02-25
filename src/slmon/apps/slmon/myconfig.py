import ConfigParser
import xml.dom.minidom

def parseXMLnode(root):
    """
    Parses an XML tree starting from root node and returns a list of
    tuples containing name, attributes and content of all child nodes.
    """

    nodes = []

    if root.hasChildNodes():
        for node in [ node for node in root.childNodes
                            if node.nodeType==node.ELEMENT_NODE ]:

            ncn = len(node.childNodes)
            if   ncn==0:
                content = None
            elif ncn==1 and node.firstChild.nodeValue:
                content = node.firstChild.nodeValue.strip()
            else:
                content = parseXMLnode(node)

            attrs = {}
            if node.hasAttributes():
                for i in xrange(node.attributes.length):
                    attr = node.attributes.item(i)
                    name = attr.nodeName
                    attrs[name] = attr.nodeValue.strip()

            nodes.append((node.nodeName, attrs, content))

    return nodes

def parseXMLfile(f):
    root = xml.dom.minidom.parse(f)
    x = parseXMLnode(root)
    if len(x)==1:
        return x[0]
    # else not 1 root element, but that's cought by xml.dom.minidom.parse()

class MyConfig(dict):

    def __init__(self, filename):

        if   filename[-4:].lower() == ".ini":
            self.readINI(filename)
        elif filename[-4:].lower() == ".xml":
            self.readXML(filename)
        else: print("XXXXXXXXXXXXXXX")

    def readINI(self, filename):
        config = ConfigParser.ConfigParser()
        config.read(filename)

        for sec in config.sections():
            d = self[sec] = {}
            for opt in config.options(sec):
                d[opt] = config.get(sec, opt)

    def readXML(self, filename):
        # XXX XML support is only provided for testing.
        name, attrs, content = parseXMLfile(filename)
        assert name == "config"
        for name, attrs, content in content:
            assert "name" in attrs
            sec = attrs["name"]
            assert name == "section"
            d = self[sec] = {}

            for name, attrs, content in content:
                if isinstance(content, list):
                    raise TypeError, "<%s> elements can't have children" % name

                if name == "string":
                    tmp = str(content)
                elif name == "int":
                    tmp = int(content)
                elif name == "float":
                    tmp = float(content)
                else:
                    raise NameError, "illegal tag '%s'" % name

                if not "name" in attrs:
                    raise NameError, "missing 'name' attribute in <%s>" % name
                opt = attrs["name"]
                d[opt] = tmp


class ConfigINI(dict):

    def __init__(self, filename, mandatory=None):
        self.read(filename)
        if not isinstance(mandatory,list):
            mandatory = []
        self.mandatory = mandatory

    def read(self, filename):
        config = ConfigParser.ConfigParser()
        config.read(filename)

        for sec in config.sections():
            d = self[sec] = {}
            for opt in config.options(sec):
                d[opt] = config.get(sec, opt)

    def fillDefault(self, defaultSection="default"):
        default = self[defaultSection]

        for section in self:
            if section == defaultSection:
                continue

            # for missing items, use the default
            for item in default:
                if item not in self[section]:
                    self[section][item] = default[item]

#           # check for items that don't appear in the default
#           for item in self[section]:
#               if item not in default and item not in self.mandatory:
#                   msg("[%s]: unmatched item '%s'" % (section, item))

            for item in self.mandatory:
                if item not in self[section]:
                    msg("[%s]: missing item   '%s'" % (section, item))
                    # XXX this could also be treated as a fatal error


class ConfigXML(MyConfig):

    def __init__(self, filename):
        self.read(filename)

    def read(self, filename):
        # XXX XML support is only provided for testing.
        name, attrs, content = parseXMLfile(filename)
        assert name == "config"
        for name, attrs, content in content:
            assert "name" in attrs
            sec = attrs["name"]
            assert name == "section"
            d = self[sec] = {}

            for name, attrs, content in content:
                if isinstance(content, list):
                    raise TypeError, "<%s> elements can't have children" % name

                if name == "string":
                    tmp = str(content)
                elif name == "int":
                    tmp = int(content)
                elif name == "float":
                    tmp = float(content)
                else:
                    raise NameError, "illegal tag '%s'" % name

                if not "name" in attrs:
                    raise NameError, "missing 'name' attribute in <%s>" % name
                opt = attrs["name"]
                d[opt] = tmp


if __name__ == '__main__':
    for f in "test.ini", "test.xml":
        print("#### filename=%s" % f)
        config = MyConfig(f)
        print(config)
        for section in config:
            print(section, config[section])

