#
# Populate an SQLite database from Arclink webreqlog statistics HTML pages.
#
import glob
import email
import os
import sqlite3
import sys
import xml.etree.ElementTree as ET
import xml

DEBUG = False
verbose = False

# ----------- SQLite stuff ---------

def new_table(con):
    """Establish new SQLite tables."""
    with con:
        cur = con.cursor()
        linef = ""
        with open('reqlogstats.sql', 'r') as fid:
            for line in fid.readlines():
                line1 = line.strip()
                if line1.startswith('/*') or line1.startswith('*'):
                    continue
                linef += ' ' + line1
                if line1 == ')' or line1.endswith(';'):
                    #print linef
                    cur.execute(linef)
                    linef = ""


# ---------- HTML parsing/scraping stuff --------
def parse_table(table):
    """Parse <table> and return data as list of tuples.

    table - ET.Element

    """
    if table is None:
        return

    # First/any row:
    tr = table.find("thead").find("tr")
    headers = []
    for e in tr:
        if e.tag == "th":
            headers.append(e.text)
    result = [tuple(headers)]

    tbody = table.find("tbody")
    for tr in tbody:
        rowdata = []
        for td in tr:
            if td.text is None:
                link = td.find("a")
                if link is not None:
                    content = link.text
                else:
                    content = "??"
            else:
                if td.attrib.has_key("sorttable_customkey"):
                    content = td.attrib["sorttable_customkey"]
                else:
                    content = td.text
            try:
                rowdata.append(content.strip().encode('ascii', 'replace'))
            except UnicodeEncodeError as e:
                print "While working on row", len(result), "of table with headers:", "|".join(headers)
                print "ERROR working on content: {", content, "}"
                raise e

            #print '|'.join([td.tag, str(td.attrib), str(content).strip(), str(td.tail)])
        result.append(tuple(rowdata))
    return result

def parse_intro_text(text, attribs):
    """Handles all text between <h2>Arclink...Report</h2> and the first <table>."""

    for line in text.splitlines():
        parts = line.split(":", 1)
        if len(parts) > 1:
            tag = parts[0].strip().replace(" ", "")
            rest = parts[1].strip()
            if tag == "Start" or tag == "End":
                rest = " ".join(rest.split(" ", 2)[0:2])
            attribs[tag] = rest

def parse_html(data):
    """Extract everything of value from a report.

    data - a strung, not a file!

    result - a list of table dumps, plus any extra non-table stuff as a dictionary.

    """
    attribs = {}
    result = [attribs]

    parser = ET.XMLParser(html=1)
    try:
        parser.feed(data)
    #except (xml.parsers.expat.ExpatError, xml.etree.ElementTree.ParseError) as e:
    except (xml.parsers.expat.ExpatError) as e:
        print "Parser ExpatError while reading the file."
        print e
        return None

    except:  ###  ParseError as e:
        print "Error while reading the file."
        raise
    elem = parser.close()
    body = elem.find("body")
    if body is None:
        print "Got no body!"

    parent = body
    table = parent.find("table")
    if table is None:
        bodydiv = parent.find("div")
        #print "bodydiv:",  bodydiv, bodydiv.attrib
        #for e in bodydiv:
        #    print '('+e.tag+')'
        table = bodydiv.find("table")

        if table is None:
            prediv = bodydiv.find("pre")
            table = prediv.find("table")
            parent = prediv

    heading = parent.find("h2")
    if heading is not None:
        # Preamble contains mark-up. This would be too easy:
        #preamble = heading.tail

        # Instead slurp all until first table:
        # What's the right way to do this?
        for t in parent:
            if t.tag == "table":
                break
            if t.tag == "h2":
                preamble = t.tail
            else:
                if t.tag == "a":
                    preamble += t.text + t.tail
                else:
                    preamble += ET.tostring(t)  # Includes t's tail. + t.tail

        if (DEBUG):
            print 50*"="
            print preamble
            print 50*"="

        parse_intro_text(preamble, attribs)
        #print "Preamble: got", attribs

    verbose = False
    things = parent.findall("table")
    if (verbose):
        print "Num <table>s:", len(things)
    for table in things:
        if (verbose):
            print table.tag, table.attrib,
        stuff = parse_table(table)
        if (verbose):
            print '\t|'.join(stuff[0])
        #print '\t+'.join(len(headers)*['-------'])
        #for row in stuff:
        #    print '\t|'.join(row)

        result.append(stuff)
    return result

# ---------- Connecting stuff -----------
class ReportData(object):
    """Holds the contents of a report, but not its key."""

    summary = {}
    user = {}
    request = {}
    volume = {}
    station = {}  # Not included in current reports, Dec 2013?
    network = {}
    messages = {}
    clientIP = {}
    userIP = {}

    def __init__(self, text):
        content = parse_html(text)
        if content == None:
            return  ## an empty object

        attribs = content[0]
        tables = content[1:]

        if attribs.has_key('TotalWaveformSize'):
            ts = attribs['TotalWaveformSize']
        elif attribs.has_key('TotalSize'):
            ts = attribs['TotalSize']  # Not present after Jan 2014?
        else:
            ts = "0"
        self.summary = {'requests': attribs['Requests'],
                   'request_with_errors': attribs['withErrors'],
                   'error_count': attribs['ErrorCount'],
                   'users': attribs['Users'],
                   'lines': attribs['TotalLines'],
                   'size': ts,  
                   'start': attribs['Start'],
                   'end': attribs['End']
                   }
        if attribs.has_key("Stations"):
            self.summary["stations"] = attribs["Stations"]

        table = tables[0]
        assert table[0][0] == "User"
        self.user = table

        for table in tables:
            if table[0][0] == "Station":
                self.station = table

            if table[0][0] == "Network":
                self.network = table

            if table[0][0] == "Request Type":
                self.request = table

                # Three of these, type = { WAVEFORM | ROUTING | INVENTORY }
                #request = {'type': t,
                #    'requests': r,
                #    'lines': l,
                #    'errors': e,
                #    'size': s
                #}

            if table[0][0] == "Volume":
                self.volume = table
                #volume = {'type': t,
                #    'requests': r,
                #    'lines': l,
                #    'errors': e,
                #    'size': s
                #    }

            if table[0][1] == "Message":
                self.messages = table

            if table[0][0] == "UserIP":
                self.userIP = table

            if table[0][0] == "ClientIP":
                self.clientIP = table



def lookup_source(con, host, port, dcid, description):
    """Return a source id - either an existing one, or a new one.

        con - Database connection cursor?
        host - varchar
        port - int
        dcid - varchar
        description - what sort of source is this?

    Returns an int, index into db tables.

    """

    constraints = " WHERE " + " AND ".join(["host = '%s'" % host,
                                               "port = %i" % port,
                                               "dcid = '%s'" % dcid])

    cursor = con.cursor()
    q = "SELECT COUNT(*) FROM `ArcStatsSource`" + constraints
    #print "SQLITE: %s" % q
    cursor.execute(q)
    result = cursor.fetchall()  # Not present: get [(0,)]
    found = (result[0][0] != 0)
    #print result, 'found=', found
    if found == False:
        q = "INSERT INTO ArcStatsSource (host, port, dcid, description) VALUES (?, ?, ?, ?)"
        v = (host, port, dcid, description)
        print "SQLITE: %s" % q
        cursor.execute(q, v)
        result = cursor.fetchall()
        con.commit()

    q = "SELECT id FROM `ArcStatsSource`" + constraints
    #print "SQLITE: %s" % q
    cursor.execute(q)
    result = cursor.fetchall()
    #print result
    assert len(result) == 1

    cursor.close()

    return int(result[0][0])

def has_summary(con, k):
    cursor = con.cursor()
    q = "SELECT COUNT(*) FROM `ArcStatsSummary` WHERE start_day = ? AND src = ?"
    cursor.execute(q, k)
    result = cursor.fetchall()  # Not present: get [(0,)]
    assert len(result) == 1
    
    found = (result[0][0] != 0)
    return found

def insert_summary(con, k, summary):
    cursor = con.cursor()
    try:
        r = summary['requests']
        rwe = summary['request_with_errors']
        e = summary['error_count']
        u = summary['users']
        tl = summary['lines']
        ts = summary['size']

        if summary.has_key('stations'):
            st = summary['stations']
            q = "INSERT INTO ArcStatsSummary (start_day, src, requests, requests_with_errors, error_count, users, stations, total_lines, total_size) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"
            v = (k[0], k[1], r, rwe, e, u, st, tl, ts)
        else:
            q = "INSERT INTO ArcStatsSummary VALUES (?, ?, ?, ?, ?, ?, NULL, ?, ?)"
            v = (k[0], k[1], r, rwe, e, u, tl, ts)

    except KeyError:
        print "Couldn't find some needed value(s)"
        for k, v in summary.iteritems():
            print k,':', v
        return

    try:
        cursor.execute(q, v)
        con.commit()
    except sqlite3.IntegrityError as e:
        print "In insert_summary:"
        print "Start day:", k[0]
        print "Source:   ", k[1]
        print e
       

def report_insert(tablename, heads):
    global verbose
    if (verbose):
        print "%s table: got" % (tablename), "|".join(heads)
    else:
        print "%s " % (tablename),


def insert_user(con, k, user):
    cursor = con.cursor()
    q = "INSERT INTO ArcStatsUser VALUES (?, ?, ?, ?, ?, ?, ?)"
    heads = user[0]
    report_insert('User', heads)

    for row in user[1:]:
        items = row[0:5]  # User  Requests  Lines  Nodata/Errors  Size
        v = (k[0], k[1], items[0], items[1], items[2], items[3], items[4])
        cursor.execute(q, v)
    con.commit()

def insert_request(con, k, table):
    cursor = con.cursor()
    q = "INSERT INTO ArcStatsRequest VALUES (?, ?, ?, ?, ?, NULL, ?, NULL)"
    heads = table[0]
    report_insert('Request', heads)
    for row in table[1:]:
        items = row[0:4]  # Request Type    Requests    Lines    Nodata/Errors
        v = (k[0], k[1], items[0], items[1], items[2], items[3])
        cursor.execute(q, v)
    con.commit()

def insert_volume(con, k, table):
    cursor = con.cursor()
    q = "INSERT INTO ArcStatsVolume VALUES (?, ?, ?, ?, NULL, ?, ?)"
    heads = table[0]
    report_insert('Volume', heads)

    for row in table[1:]:
        items = row[0:4]  # Volume    Count    Nodata/Errors    Size
        v = (k[0], k[1], items[0], items[1], items[2], items[3])
        cursor.execute(q, v)
    con.commit()

def insert_station(con, k, table):
    heads = table[0]
    report_insert('Station', heads)
    print ' ...IGNORED'

def insert_network(con, k, table):
    cursor = con.cursor()
    q = "INSERT INTO ArcStatsNetwork VALUES (?, ?, ?, ?, ?, ?, ?, ?, NULL)"
    heads = table[0]
    report_insert('Network', heads)

    merged_errors_nodata = False;
    if heads[3] == "Errors/Nodata":
        # Old-style report from before 2013-12-03 or so,
        # store this column's data under 'Errors' in the db.
        merged_errors_nodata = True;

    for row in table[1:]:
        if len(row) == 6:
            items = row[0:6]  # Network    Requests    Lines    Nodata    Errors    Size
        elif (len(row) == 5) and merged_errors_nodata:
            items = [row[0], row[1], row[2], 0, row[3], row[4]]
        else:
            print "Funny row, skipping it:", row
            continue

        v = (k[0], k[1], items[0], items[1], items[2], items[3], items[4], items[5])
        cursor.execute(q, v)
    con.commit()

def insert_messages(con, k, table):
    cursor = con.cursor()
    q = "INSERT INTO ArcStatsMessages VALUES (?, ?, ?, ?)"
    heads = table[0]
    report_insert("Messages", heads)

    for row in table[1:]:
        items = row[0:2]  # Count    Message
        v = (k[0], k[1], items[1], items[0])
    cursor.execute(q, v)
    con.commit()

def insert_clientIP(con, k, table):
    cursor = con.cursor()
    heads = table[0]
    report_insert("ClientIP", heads)
    q = "INSERT INTO ArcStatsClientIP VALUES (?, ?, ?, ?, ?, ?, NULL)"

    for row in table[1:]:
        items = row[0:4]  # ClientIP    Requests    Lines    Nodata/Errors
        v = (k[0], k[1], items[0], items[1], items[2], items[3])
        cursor.execute(q, v)
    con.commit()

def insert_userIP(con, k, table):
    cursor = con.cursor()
    heads = table[0]
    report_insert("UserIP", heads)
    q = "INSERT INTO ArcStatsUserIP VALUES (?, ?, ?, ?, ?, ?, NULL)"

    for row in table[1:]:
        items = row[0:4]  # UserIP    Requests    Lines    Nodata/Errors
        v = (k[0], k[1], items[0], items[1], items[2], items[3])
        cursor.execute(q, v)
    con.commit()

def insert_data(db, rd, host, port, dcid, description, start_day):
    """Insert the information found in a report into a database.

    db - database object to insert into
    rd - ReportData object

    If a report was already received, nothing is inserted.
    (In future: should replace previous data for this day and source?)

    """
    y = int(start_day[0:4]);
    if not db.endswith("-%i.db" % y):
	print " *** Watch out, start day doesn't match db file; skipping"
	return 0

    con = sqlite3.connect(db)
    source_id = lookup_source(con, host, port, dcid, description)
    k = (start_day, source_id)  ## SOME_KEY_IDENTIFYING_A_REPORT
    if has_summary(con, k):
        print " *** FOUND Existing summary for", k, "in the db; skipping"
        return 0
    
    print "Inserting tables... ",
    insert_summary(con, k, rd.summary)
    insert_user(con, k, rd.user)
    insert_request(con, k, rd.request)
    insert_volume(con, k, rd.volume)
    if rd.station: insert_station(con, k, rd.station)
    if rd.network: insert_network(con, k, rd.network)
    if len(rd.messages) > 0:
        insert_messages(con, k, rd.messages)
    if len(rd.clientIP) > 0:
        insert_clientIP(con, k, rd.clientIP)
    if rd.userIP:
        insert_userIP(con, k, rd.userIP)
    con.close()

    global verbose
    if (not verbose):
        print
    return 1

def summary_data(db):
    """Quick overview of what's in a database.

    db - string, file name of the SQLite database to examine.

    """
    summary = {}

    tables = ['Source', 'Messages', 'Report', 'Request', 'Network', 'Station', 'Summary', 'User', 'UserIP', 'Volume']
    con = sqlite3.connect(db)
    cur = con.cursor()

    for tableName in tables:
        cur.execute("SELECT COUNT(*) FROM `ArcStats%s`;" % tableName)
        result = cur.fetchall()
        summary[tableName] = result[0][0]

    con.close()
    return summary

def process_html_file(f, host, port, dcid, description):
    """
    Arguments:
     f - HTML file
    Returns:
     0 - not inserted
     1 - inserted
     3 - no summary (unparsable?)
    """
    text = open(f).read().replace("<hr>", "<hr/>")
    rd = ReportData(text)

    if len(rd.summary) == 0:
        print "%s: empty summary" % (f)
        return 3

    try:
        print "Covers %(s)s to %(e)s - %(req)s requests %(lines)s lines, size %(siz)s" % {'s': rd.summary['start'],
                                                                                          'e': rd.summary['end'],
                                                                                          'req': rd.summary['requests'],
                                                                                          'lines': rd.summary['lines'],
                                                                                          'siz': rd.summary['size']}
    except KeyError as e:
        print "Error reading summary object", e
        print rd.summary
    start_day = rd.summary['start'].split(' ', 1)[0]
    return insert_data(db, rd, host, port, dcid, description, start_day)

# This directory must match where the PHP report
# generator looks for SQLite DB files:
reqlogstats_db_dir="/home/sysop/reqlogstats/var"
if (not os.path.isdir(reqlogstats_db_dir)):
    print " *** Configuration error: %s is not an existing directory." % (reqlogstats_db_dir)
    raise IOError, 'No such directory.'

# Which database file gets used should depend on the start date
# found in the report! As a workaround, allow overriding on
# command line.
import datetime
year = datetime.datetime.now().year
if len(sys.argv) > 1 and int(sys.argv[1]) > 2010:
    year = int(sys.argv[1])

db = os.path.join(reqlogstats_db_dir, 'reqlogstats-%4i.db' % (year))
scores = 4*[0]  # [0, 0]
scores_labels = ("rejected", "inserted", "not found", "unparseable")
unparsed_list = []

if os.path.exists(db):
    con = sqlite3.connect(db)
    # Only works if the tables exist:
    #print summary_data(db)
else:
    print "Creating new database file", db
    con = sqlite3.connect(db)
    new_table(con)

# Now look at the report file(s),
# and import their contents into our db.
if (DEBUG):
    count = 0
    for line in con.iterdump():
        count += 1
        print 'DUMP %03i:' % count, line


filelist = []

# Given input file(s) as a list of names on stdin, insert them:
#filelist = glob.glob("./eida_stats/2014/01/*")
#filelist.extend(glob.glob("./eida_stats/2014/02/*"))
for line in sys.stdin.readlines():
	filelist.append(line.strip())

bodyfile="/tmp/reqlogstats.txt"

source_dict = {"bgr": "BGR",
               "ethz": "ETHZ",
               "gfz-potsdam": "GFZ",
               "ingv": "INGV",
               "ipgp": "IPGP",
               "resif": "RESIF",
               "knmi": "ODC",
               "uni-muenchen": "LMU"
              }

for myfile in filelist:
    print 70*'-'

    if not os.path.exists(myfile):
	scores[2] += 1
	continue

    with open(myfile, 'r') as fid:
        msg = email.message_from_file(fid)
    who = msg['From']

    # Heuristic to set DCID/source string from From:
    emailaddr = email.utils.parseaddr(who)[1]  # Discard "realname" part
    try:
        host = emailaddr.split('@')[1]
        d = host.split('.')[-2]
    except IndexError:
    	host = emailaddr
	d = emailaddr    #???    

    port = -1

    try:
        dcid = source_dict[d.lower()]
    except KeyError:
        dcid = emailaddr.lower()  ## Use the sender's name

    print "Processing %s from %s: %s (%s:%i)" % (myfile, emailaddr, dcid, host, port)

    with open(bodyfile, 'w') as fid:
	buf = msg.get_payload()
	# Replacements to make HTML pile of tags look like XHTML.
	buf = buf.replace('""', '&quot;&quot;');
        print >>fid, buf.replace("<hr>", "<hr />").replace("<br>", "<br />")
    result = process_html_file(bodyfile, host, port, dcid, 'E-mail from ' + emailaddr)
    os.unlink(bodyfile)
    scores[result] += 1

    if (result == 3):
	unparsed_list.append(myfile)


print 70*'-'
print "Done with %i file(s)." % len(filelist)
print "Scores:",
for k in range(len(scores)):
    print " %i %s," % (scores[k], scores_labels[k]),
print

summary = summary_data(db)
for (k, v) in summary.items():
    print k ,":", v
print "Database %s contains  %i source(s), and %i day summaries." % (db, summary['Source'], summary['Summary'])
if (len(unparsed_list) > 0):
   for f in sorted(unparsed_list):
	print "Unparsed file", f

