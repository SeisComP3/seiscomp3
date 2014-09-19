#***************************************************************************** 
# iniparser.py
#
# Parser for reading SeisComP .ini files
#
# (c) 2004 Andres Heinloo, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import re
from seiscomp import logs

_rx_ini_space = re.compile(r'\s+')
_rx_ini_comment = re.compile(r'[\*#;]')
_rx_ini_section = re.compile(r'\[\s*([^\s\[\]]+)\s*\]')
_rx_ini_definition = re.compile(r'([^\s\[\]=]+)\s+([^\s=]+)')
_rx_ini_qassignment = re.compile(r'([^\s\[\]=]+)\s*=\s*"(([^\\"]|\\.)*)"')
_rx_ini_assignment = re.compile(r'([^\s\[\]=]+)\s*=\s*([^\s=]+)')

class _ci_str(str):
    def __hash__(self):
        return str.__hash__(self.lower())

    def __eq__(self, other):
        return str.__eq__(self.lower(), other.lower())

    def __ge__(self, other):
        return str.__ge__(self.lower(), other.lower())

    def __gt__(self, other):
        return str.__gt__(self.lower(), other.lower())

    def __le__(self, other):
        return str.__le__(self.lower(), other.lower())

    def __lt__(self, other):
        return str.__lt__(self.lower(), other.lower())

    def __ne__(self, other):
        return str.__ne__(self.lower(), other.lower())

class _ci_dict(dict):
    def __contains__(self, key):
        return dict.__contains__(self, _ci_str(key))
        
    def __delitem__(self, key):
        return dict.__delitem__(self, _ci_str(key))
        
    def __getitem__(self, key):
        return dict.__getitem__(self, _ci_str(key))
        
    def __setitem__(self, key, value):
        return dict.__setitem__(self, _ci_str(key), value)

    def has_key(self, key):
        return dict.has_key(self, _ci_str(key))

    def get(self, key, default=None):
        return dict.get(self, _ci_str(key), default)

def read_ini(source):
    if isinstance(source, basestring):
        fp = open(source)
    elif hasattr(source, "read"):
        fp = source
    else:
        raise TypeError, "invalid source"

    try:
        filename = fp.name
    except AttributeError:
        filename = '<???>'

    lineno = 0
    pos = 0
    
    def warning(s):
        logs.warning("%s:%d:%d: %s" % (filename, lineno, pos + 1, s))
    
    in_section = False
    in_definition = False
    skip_section = 0
    skip_definition = 1
    skip = None
    dict_stack = [_ci_dict()]
    while True:
        line = fp.readline()
        if not line:
            break

        lineno = lineno + 1
        pos = 0
        while pos < len(line):
            if pos == 0 and _rx_ini_comment.match(line):
                break
            
            m = _rx_ini_space.match(line, pos)
            if m != None:
                pos = m.end()
                continue

            m = _rx_ini_section.match(line, pos)
            if m != None:
                skip = None

                if in_definition:
                    dict_stack.pop()
                    in_definition = False
                    
                if in_section:
                    dict_stack.pop()
                    in_section = False

                if not dict_stack[-1].has_key(m.group(1)):
                    chd = _ci_dict()
                    dict_stack[-1][m.group(1)] = chd
                    dict_stack.append(chd)
                    in_section = True
                else:
                    warning("section '%s' appears multiple times" % \
                        (m.group(1),))
                    skip = skip_section

                pos = m.end()
                continue

            m = _rx_ini_definition.match(line, pos)
            if m != None:
                if skip == None or skip >= skip_definition:
                    skip = None
                    if not in_section:
                        warning("parse error")
                        break

                    if in_definition:
                        dict_stack.pop()
                        in_definition = False
                        
                    d = dict_stack[-1].get(m.group(1))
                    if d == None:
                        d = _ci_dict()
                        dict_stack[-1][m.group(1)] = d
                    
                    if isinstance(d, dict):
                        if not d.has_key(m.group(2)):
                            chd = _ci_dict()
                            d[m.group(2)] = chd
                            dict_stack.append(chd)
                            in_definition = True
                        else:
                            warning("%s '%s' is already defined" % m.groups())
                            skip = skip_definition
                    else:
                        warning("keyword '%s' conflicts with a parameter" % \
                            (m.group(1),))
                        skip = skip_definition

                pos = m.end()
                continue

            m = _rx_ini_qassignment.match(line, pos)
            if m != None:
                if skip == None:
                    if not in_section:
                        warning("parse error")
                        break

                    if not dict_stack[-1].has_key(m.group(1)):
                        dict_stack[-1][m.group(1)] = m.group(2).replace(r'\"', r'"')
                    else:
                        warning("parameter '%s' is assigned multiple times" % \
                            (m.group(1),))
                    
                pos = m.end()
                continue

            m = _rx_ini_assignment.match(line, pos)
            if m != None:
                if skip == None:
                    if not in_section:
                        warning("parse error")
                        break

                    if not dict_stack[-1].has_key(m.group(1)):
                        dict_stack[-1][m.group(1)] = m.group(2)
                    else:
                        warning("parameter '%s' is assigned multiple times" % \
                            (m.group(1),))
                    
                pos = m.end()
                continue

            warning("parse error")
            break

    fp.close()
    return dict_stack[0]
 
