#!/usr/bin/env python

from    getopt  import getopt, GetoptError
from    time    import time, gmtime
from    datetime import datetime
import  os, sys, signal, glob, re
from    seiscomp.myconfig import MyConfig
import  seiscomp.slclient

ini_stations, ini_setup = "stations.ini", "setup.ini"

verbose = 0

class Status:

    def __repr__(self):
        return "%2s %-5s %2s %3s %1s %s %s" % \
                        (self.net, self.sta, self.loc, self.cha, self.typ, \
                            str(self.last_data), str(self.last_feed))
class StatusDict(dict):

    def __init__(self, source=None):
        if source:
            self.read(source)

    def fromSlinkTool(self,server="",stations=["GE_MALT","GE_MORC","GE_IBBN"]):
        # later this shall use XML
        cmd = "slinktool -nd 10 -nt 10 -Q %s" % server
        print cmd
        f = os.popen(cmd)
        regex = re.compile("[SLBVEH]H[ZNE]")
        for line in f.xreadlines():
            net_sta = line[:2].strip() + "_" + line[3:8].strip()
            if not net_sta in stations:
                continue
            typ = line[16]
            if typ != "D":
                continue
            cha = line[12:15].strip()
            if not regex.match(cha):
                continue

            d = Status()
            d.net = line[ 0: 2].strip()
            d.sta = line[ 3: 8].strip()
            d.loc = line[ 9:11].strip()
            d.cha = line[12:15]
            d.typ = line[16]
            d.last_data = seiscomp.slclient.timeparse(line[47:70])
            d.last_feed = d.last_data
            sec = "%s_%s" % (d.net, d.sta)
            sec = "%s.%s.%s.%s.%c" % (d.net, d.sta, d.loc, d.cha, d.typ)
            self[sec] = d

    def read(self, source):
        if type(source) == str:
            source = file(source)
        if type(source) == file:
            source = source.readlines()
        if type(source) != list:
            raise TypeError, 'cannot read from %s' % str(type(source))

        for line in source:
            d = Status()
            d.net = line[ 0: 2]
            d.sta = line[ 3: 8].strip()
            d.loc = line[ 9:11].strip()
            d.cha = line[12:15]
            d.typ = line[16]
            d.last_data = seiscomp.slclient.timeparse(line[18:41])
            d.last_feed = seiscomp.slclient.timeparse(line[42:65])
            if  d.last_feed < d.last_data:
                d.last_feed = d.last_data
            sec = "%s_%s:%s.%s.%c" % (d.net, d.sta, d.loc, d.cha, d.typ)
            self[sec] = d

    def write(self, f):
        if type(f) is str:
            f = file(f, "w")
        lines = []
        for key in self.keys():
            lines.append(str(self[key]))
        lines.sort()
        f.write('\n'.join(lines)+'\n')


def colorLegend(htmlfile):
    htmlfile.write("<p><center>Latencies:<br>\n" \
        "<table cellpadding='2' cellspacing='1' border='0'" \
	      " bgcolor='#000000'>\n<tr>\n" \
        "<td bgcolor='#cc99ff'>&nbsp;&lt;30 m&nbsp;</td>\n" \
        "<td bgcolor='#3399ff'>&nbsp;&lt; 1 h&nbsp;</td>\n" \
        "<td bgcolor='#00ff00'>&nbsp;&lt; 2 h&nbsp;</td>\n" \
        "<td bgcolor='#ffff00'>&nbsp;&lt; 6 h&nbsp;</td>\n" \
        "<td bgcolor='#ff9966'>&nbsp;&lt; 1 d&nbsp;</td>\n" \
        "<td bgcolor='#ff3333'>&nbsp;&lt; 2 d&nbsp;</td>\n" \
        "<td bgcolor='#ffcccc'>&nbsp;&lt; 3 d&nbsp;</td>\n" \
        "<td bgcolor='#cccccc'>&nbsp;&lt; 4 d&nbsp;</td>\n" \
        "<td bgcolor='#999999'>&nbsp;&lt; 5 d&nbsp;</td>\n" \
        "<td bgcolor='#666666'>&nbsp;&gt; 5 d&nbsp;</td>\n" \
        "</tr>\n</table>\n</center></p>\n")

# encodes an email address so that it cannot (easily) be extracted
# from the web page. This is meant to be a spam protection.
def encode(txt): return ''.join(["&#%d;" % ord(c) for c in txt])

def total_seconds(td): return td.seconds + (td.days*86400)

def pageTrailer(htmlfile, config):
    
    htmlfile.write("<hr>\n" \
        "<table width='99%%' cellpaddding='2' cellspacing='1' border='0'>\n" \
            "<tr>\n<td>Last updated %04d/%02d/%02d %02d:%02d:%02d UTC</td>\n" \
            "    <td align='right'><a href='%s' " \
            "target='_top'>%s</a></td>\n</tr>\n" \
        "</table>\n</body></html>\n" % (gmtime()[:6]  +  (config['setup']['linkurl'],) +  (config['setup']['linkname'],)) )

def getColor(delta):
    delay = total_seconds(delta)
    if   delay >432000: return '#666666'
    if   delay >345600: return '#999999'
    if   delay >259200: return '#cccccc'
    if   delay >172800: return '#ffcccc'
    if   delay > 86400: return '#ff3333'
    elif delay > 21600: return '#ff9966'
    elif delay >  7200: return '#ffff00'
    elif delay >  3600: return '#00ff00'
    elif delay >  1800: return '#3399ff'
    else:               return '#cc99ff'

TDdummy = "<td align='center' bgcolor='%s'><tt>n/a</tt></td>"

def TDf(delta, col="#ffffff"):
    if delta is None: return TDdummy % col

    t = total_seconds(delta)

    if   t > 86400: x = "%.1f d" % (t/86400.)
    elif t >  7200: x = "%.1f h" % (t/3600.)
    elif t >   120: x = "%.1f m" % (t/60.)
    else:           x = "%.1f s" % (t)
    return "<td align='right' bgcolor='%s'><tt> &nbsp;%s&nbsp;</tt></td>" % \
                (col,x)

def TDt(t, col="#ffffff"):
    if t is None: return TDdummy % col

    x = t.strftime("%Y/%m/%d %H:%M:%S")
    return "<td align='center' bgcolor='%s'><tt>&nbsp;%s&nbsp;</tt></td>" % \
                (col,x)

def myrename(name1, name2):

    # fault-tolerant rename that doesn't cause an exception if it fails, which
    # may happen e.g. if the target is on a non-reachable NFS directory
    try:
        os.rename(name1, name2)
    except OSError:
        print "failed to rename(%s,%s)" % (name1, name2)


def makeMainHTML(config):

    global status

    now = datetime.utcnow()

    stations = []

    streams = [ x for x in status.keys() if x.find("BHZ")>0 or
        x.find("SHZ")>0 or x.find("EHZ")>0 ]

    streams.sort()

    tmp_rt = []
    tmp_du = []

    for label in streams:
        lat1 = now - status[label].last_data # XXX
        lat2 = now - status[label].last_feed # XXX
        lat3 = lat1-lat2 # XXX
        if lat3 == 0.: lat3 = lat2 = None

        if label[-2]=='.' and label[-1] in "DE":
            label = label[:-2]
        n,s,x,x = label.split(".")
        if s in stations: continue # avoid duplicates for different locations
        stations.append(s)

        net_sta = "%s_%s" % (n,s)
        line = "<tr bgcolor='#ffffff'><td><tt>&nbsp;%s <a " \
		"href='%s.html'>%s</a>&nbsp;</td>%s%s%s</tr>" \
	        % (n, net_sta, s, TDf(lat1, getColor(lat1)),
                                  TDf(lat2, getColor(lat2)),
                                  TDf(lat3, getColor(lat3)))
	if config.station[net_sta]['type'][:4] == 'real':
	        tmp_rt.append(line)
	else:   tmp_du.append(line)
	makeStatHTML(net_sta, config)

    try: os.makedirs(config['setup']['wwwdir'])
    except: pass

    temp = "%s/tmp.html"   % config['setup']['wwwdir']
    dest = "%s/index.html" % config['setup']['wwwdir']
    
    table_begin = """
    <table cellpaddding='2' cellspacing='1' border='0' bgcolor='#000000'>
    <tr>
      <th bgcolor='#ffffff' rowspan='2' align='center'>Station</th>
      <th bgcolor='#ffffff' colspan='3' align='center'>Latencies</th>
    </tr>
    <tr>
      <th bgcolor='#ffffff' align='center'>Data</th>
      <th bgcolor='#ffffff' align='center'>Feed</th>
      <th bgcolor='#ffffff' align='center'>Diff.</th>
    </tr>
    """
    table_end = """
    </table>
    """

    htmlfile = file(temp, "w")
    htmlfile.write("""<html>
    <head>
        <title>%s</title>
        <meta http-equiv='refresh' content='%d'>
	<link rel='SHORTCUT ICON' href='%s'>
    </head>
    <body bgcolor='#ffffff'>
    <center><font size='+2'>%s</font></center>\n""" % \
            (   config['setup']['title'], int(config['setup']['refresh']),
                config['setup']['icon'],      config['setup']['title']))


    htmlfile.write("<center><table cellpaddding='5' cellspacing='5'><tr>\n")
    if len(tmp_rt):
        htmlfile.write("<td valign='top' align='center'>\n" \
		       "<font size='+1'>Real-time stations<font>\n</td>\n")
    if len(tmp_du):
        htmlfile.write("<td valign='top' align='center'>\n" \
		       "<font size='+1'>Dial-up stations<font>\n</td>\n")
    htmlfile.write("</tr><tr>")
    if len(tmp_rt):
	htmlfile.write("<td valign='top' align='center'>\n")
	htmlfile.write(table_begin)
	htmlfile.write("\n".join(tmp_rt))
	htmlfile.write(table_end)
	htmlfile.write("</td>\n")
    if len(tmp_du):
	htmlfile.write("<td valign='top' align='center'>\n")
	htmlfile.write(table_begin)
	htmlfile.write("\n".join(tmp_du))
	htmlfile.write(table_end)
	htmlfile.write("</td>\n")
    htmlfile.write("</tr></table></center>\n")

    colorLegend(htmlfile)
    pageTrailer(htmlfile, config)
    htmlfile.close()
    myrename(temp, dest)


def makeStatHTML(net_sta, config):
    global status

    try: os.makedirs(config['setup']['wwwdir'])
    except: pass

    temp = "%s/tmp2.html"  % config['setup']['wwwdir']
    dest = "%s/%s.html"  % ( config['setup']['wwwdir'], net_sta)

    htmlfile = file(temp, "w")
    htmlfile.write("""<html>
        <head>
            <title>%s - Station %s</title>
            <meta http-equiv='refresh' content='%d'>
            <link rel='SHORTCUT ICON' href='%s'>
        </head>
        <body bgcolor='#ffffff'>
            <center><font size='+2'>%s - Station %s</font>\n""" % \
            (   config['setup']['title'], net_sta, int(config['setup']['refresh']),
                config['setup']['icon'],
                config['setup']['title'], net_sta.split("_")[-1]))
        
    try:
        name = config.station[net_sta]['info']
        htmlfile.write("<br><font size='+1'>%s</font>" % name)
    except: pass
    htmlfile.write("</center>\n")

    if 'text' in config.station[net_sta]:
        htmlfile.write("<P>%s</P>\n" % config.station[net_sta]['text'])

    htmlfile.write("""<p><center>
    <table cellpadding='2' cellspacing='1' border='0' bgcolor='#000000'>
    <tr>
      <th bgcolor='#ffffff' align='center' rowspan='2'>Station/<br>Channel</th>
      <th bgcolor='#ffffff' align='center' colspan='2'>Data</th>
      <th bgcolor='#ffffff' align='center' colspan='2'>Feed</th>
      <th bgcolor='#ffffff' align='center' rowspan='2'>Diff.</th>
    </tr>
    <tr>
      <th bgcolor='#ffffff' align='center'>Last Sample</th>
      <th bgcolor='#ffffff' align='center'>Latency</th>
      <th bgcolor='#ffffff' align='center'>Last Received</th>
      <th bgcolor='#ffffff' align='center'>Latency</th>
    </tr>""")

    now = datetime.utcnow()

    netsta2=net_sta.replace("_",".")
    streams = [ x for x in status.keys() if x.find(netsta2)==0 ]
    streams.sort()
    for label in streams:
        tim1 = status[label].last_data
        tim2 = status[label].last_feed

        lat1, lat2, lat3 = now-tim1, now-tim2, tim2-tim1
        col1, col2, col3 = getColor(lat1), getColor(lat2), getColor(lat3)
        if lat1==lat2: lat2 = lat3 = None
        if label[-2]=='.' and label[-1] in "DE":
            label = label[:-2]
        n,s,loc,c = label.split(".")
        c = ("%s.%s" % (loc,c)).strip(".")
        htmlfile.write("<tr bgcolor='#ffffff'><td>" \
                       "<tt>&nbsp;%s %s&nbsp;</td>%s%s%s%s%s</tr>\n" \
            % (s, c, TDt(tim1, col1), TDf(lat1, col1),
                     TDt(tim2, col2), TDf(lat2, col2),
                     TDf(lat3, col3)))

    htmlfile.write("</table></p>\n")
    colorLegend(htmlfile)
    
    htmlfile.write("<p>\nHow to <a href='http://geofon.gfz-potsdam.de/waveform/status/latency.php' target='_blank'>interpret</a> " \
		    "these numbers?<br>\n")
    if 'liveurl' in config['setup']:
	# substitute '%s' in live_url by station name
	url = config['setup']['liveurl'] % s
	htmlfile.write("View a <a href='%s' target='_blank'>live seismogram</a> of "
			"station %s</center>\n" % (url, s))
    htmlfile.write("</p>\n")
    pageTrailer(htmlfile, config)
    htmlfile.close()
    myrename(temp, dest)

def read_ini():
    global config, ini_setup, ini_stations
    print "reading setup config from '%s'" % ini_setup
    config = MyConfig(ini_setup)
    print "reading station config from '%s'" % ini_stations
    config.station = MyConfig(ini_stations)

def SIGINT_handler(signum, frame):
    global status
    print "received signal #%d => will write status file and exit" % signum
#   status.write("status.tab")
    sys.exit(0)


try:
    opts, args = getopt(sys.argv[1:], "c:s:t:hv")
except GetoptError:
    usage(exitcode=2)


for flag, arg in opts:
    if flag == "-c":    ini_setup = arg
    if flag == "-s":    ini_stations = arg
    if flag == "-t":    refresh = float(arg) # XXX not yet used
    if flag == "-h":    usage(exitcode=0)
    if flag == "-v":    verbose = 1


signal.signal(signal.SIGHUP,  SIGINT_handler)
signal.signal(signal.SIGINT,  SIGINT_handler)
signal.signal(signal.SIGQUIT, SIGINT_handler)
signal.signal(signal.SIGTERM, SIGINT_handler)

read_ini()

cha = "?H?"
loc = ""

s = config.station
net_sta = ["%s_%s" % (s[k]['net'],s[k]['sta']) for k in s]
s_arg = ','.join(net_sta)
streams = [ (s[k]['net'],s[k]['sta'],loc,cha)  for k in s ]

if 'server' in config['setup']:
        server = config['setup']['server']
else:   server = "localhost"

#def read_initial(config):
#
#    for s in config.station:
#        print s,glob.glob("/home/dcop/seedlink/%s/segments/*" % s)
#        for f in glob.glob("/home/dcop/seedlink/%s/segments/*" % s):
#            print f
#
#read_initial(config)


#print "reading initial time windows from file 'status.tab'"
#status = StatusDict("status.tab")
status = StatusDict()
#if verbose: status.write(sys.stderr)


print "generating output to '%s'" % config['setup']['wwwdir']

print "getting initial time windows from SeedLink server '%s'" % server
status.fromSlinkTool(server, stations=net_sta)
if verbose: status.write(sys.stderr)

nextTimeGenerateHTML = time()

print "setting up connection to SeedLink server '%s'" % server
#cmd = "slinktool -s '?H?.D' -S '%s' -o - '%s'" % (s_arg, server)
#print cmd
#inpipe = os.popen(cmd)
#input = seiscomp.mseed.Input(inpipe)

input = seiscomp.slclient.Input(server, streams)
for rec in input:
    id = '.'.join([rec.net, rec.sta, rec.loc, rec.cha, rec.rectype])
#   if not id in status: continue # XXX XXX XXX
    try:
        status[id].last_data = rec.end_time
        status[id].last_feed = datetime.utcnow()
    except:
        continue

    if time() > nextTimeGenerateHTML:
        makeMainHTML(config)
        nextTimeGenerateHTML = time() + int(config['setup']['refresh'])
