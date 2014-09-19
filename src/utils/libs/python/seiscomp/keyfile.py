#***************************************************************************** 
# keyfile.py
#
# Keyfile reader
#
# (c) 2007 Andres Heinloo, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import re
from seiscomp import logs

_rx_keyline = re.compile(r"""(?P<p>[^=]+)=(?P<q>.)(?P<v>((?<=")([^\\"]|\\.)*)|(?<=')([^']*))(?P=q)$""")

class Keyfile(object):
    def __init__(self, source):
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

        try:
            lineno = 0
            while True:
                line = fp.readline()
                if not line:
                    break

                lineno = lineno + 1

                line = line.strip()
                if not line or line.startswith("#"): continue

                m = _rx_keyline.match(line.strip())
                if m:
                    s = m.group('p').replace('__', '.').split('_')
                    if s[0] == '':
                        s[0] = '_'

                    k =  reduce(lambda x, y: x + y[0] + y[1:].lower(), s[1:], s[0].lower())
                    
                    if m.group('q') == '"':
                        self.__dict__[k.replace('.', '_')] = m.group('v').replace(r'\"', r'"')
                    else:
                        self.__dict__[k.replace('.', '_')] = m.group('v')
                        
                else:
                    logs.error("%s:%d: parse error" % (filename, lineno))
        
        finally:
            if fp is not source:
                fp.close()

    def iteritems(self):
        for (k, v) in self.__dict__.iteritems():
            if k == "iteritems" or k[0] == '_':
                continue

            yield (k.replace('_', '.'), v)


