#!/usr/bin/env python
#
# What is this for?
#
# Copyright (C) 2013-2017
#     Helmholtz-Zentrum Potsdam - Deutsches GeoForschungsZentrum GFZ
#
# This software is free software and comes with NO WARRANTY.
#


import sqlite3


def find_dcid(con, dcid):
    q = "SELECT * FROM `ArcStatsSource` WHERE dcid='%s'" % (dcid)
    cursor = con.cursor()
    cursor.execute(q)
    result = cursor.fetchall()  # Not present: get [(0,)]
    found = (result[0][0] != 0)
    if not found:
        raise Exception('No summary found for dcid="%s"' % (dcid))
    assert len(result) == 1

    cursor.close()

    return int(result[0][0])


def summary_table(con, src):
    cols = ('start_day', 'src', 'requests', 'requests_with_errors',
            'error_count', 'users', 'stations',
            'total_lines', 'total_size')

    q = "SELECT * FROM `ArcStatsSummary` WHERE src=%i" % (src)
    cursor = con.cursor()
    cursor.execute(q)
    result = cursor.fetchall()  # Not present: get [(0,)]
    found = (result[0][0] != 0)
    if not found:
        raise Exception('No summary found for src=%i' % (src))
    print "Results found:", len(result)

    cursor.close()

    print "\t".join(cols)
    for r in result:
        print "\t".join(map(str, r))

    return True


def do_it(db, dcid):
    con = sqlite3.connect(db)
    source_id = find_dcid(con, dcid)
    print "Found source_id", source_id

    summary_table(con, source_id)
    con.close()
    print "Closed database"


do_it("var/reqlogstats-2015.db", "NIEP")
