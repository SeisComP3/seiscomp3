#!/usr/bin/python
#
# Extract messages from a mail folder, which a unique file for each
# sender and time. Looks at the "From:" (not "From") and "Date:"
# headers of the original message.
#
# Begun by Peter L. Evans, January 2014
# <pevans@gfz-potsdam.de>
#
# Copyright (C)
#     2014-2016 Helmholtz-Zentrum Potsdam - Deutsches GeoForschungsZentrum GFZ
#
# This software is free software and comes with NO WARRANTY.
#
# ----------------------------------------------------------------------

import email
import os
import time
import sys

verbose = False

if (len(sys.argv) > 1):
    base = sys.argv[1]
else:
    base = 'msg'

msg_dir = os.path.join(os.environ['HOME'], 'eida_stats')

# Usage: split_mail {basename} < mailfile


def addr_for_file(s):
    """Return something which can be used as part of a file name."""
    (realname, emailaddr) = email.utils.parseaddr(s)
    return emailaddr.replace('@', '_').replace('.', '_')


def date_for_file(s):
    """Return something which can be used as part of a file name."""
    tt = email.utils.parsedate(s)
    # FIXME: Is time zone handled appropriately?
    # return email.utils.formatdate(timestamp)
    return time.strftime("%Y-%m-%d-%H%M%S", tt), tt[0], tt[1], tt[2]


def parse_msg(buf):
    msg = email.message_from_string(buf)
    m_date = msg.get('Date')
    m_from = msg['From']
    return msg, m_date, m_from


def get_fname(m_date, m_from):
    (s, y, m, d) = date_for_file(m_date)
    return os.path.join(msg_dir, '%04d' % y,
                        '%02d' % m,
                        '-'.join([base, addr_for_file(m_from), s]))

count = 0
count_lines = 0
skipped_count = 0
written_count = 0

buf = ""

if not os.path.exists(msg_dir):
    print "No message directory", msg_dir, ". Bye."
    sys.exit(1)

for line in sys.stdin.readlines():
    count_lines += 1
    if line.startswith("From "):  # or EOF
        count += 1

        msg, m_date, m_from = parse_msg(buf)

        # fname = base + ".seq%04d" % count  ## fname = base + "." + day

        if m_date is not None:
            fname = get_fname(m_date, m_from)

            fdir = os.path.dirname(fname)
            if not os.path.exists(fdir):
                os.makedirs(fdir)

            if os.path.exists(fname):
                if (verbose):
                    print "Skipping existing file %s" % (fname)
                buf = ""
                skipped_count += 1
            else:
                print "Writing to %s (%i lines)" % (fname, count_lines)
                with open(fname, "w") as fid:
                    print >>fid, buf
                buf = ""
                written_count += 1

    buf += line

# Flush the remaining message:
if len(buf) > 0:
    msg, m_date, m_from = parse_msg(buf)
    if m_date is not None:
        fname = get_fname(m_date, m_from)

        fdir = os.path.dirname(fname)
        if not os.path.exists(fdir):
            os.makedirs(fdir)

        if os.path.exists(fname):
            if (verbose):
                print "Skipping existing file %s" % (fname)
            buf = ""
            skipped_count += 1
        else:
            print "Writing to %s (%i lines)" % (fname, count_lines)
            with open(fname, "w") as fid:
                print >>fid, buf
            buf = ""
            written_count += 1

if len(buf) > 0:
    print "split_mail: ERROR: still %i chars left in buf!!" % (len(buf))

print "split_mail: %i message(s) processed; %i skipped, %i written to %s" % (
    count, skipped_count, written_count, msg_dir)
