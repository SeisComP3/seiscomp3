#!/bin/sh

set -u

setup () {
	~/seiscomp3/bin/seiscomp start spread scmaster

	cat >rules0 </dev/null

	cat >rules1 <<EOF
Nw: 4A 2003/001
Na: archive=INGV
Na: netClass=t
Sa: archive=INGV *
EOF

	cat >rules2 <<EOF
Nw: 4A 2003/001
Na: Archive=INGV
Na: NetClass=t
Sa: Archive=INGV *
EOF
}

teardown () {
	rm -f 1 expected
	rm -f rules?
	rm -f foo.xml 4a_new.xml
}

# ----------------------------------------------------------------------

test1 () {
	echo "test1"
wget "http://geofon.gfz-potsdam.de/fdsnws/station/1/query?net=GE&cha=B*&sta=A*&start=2016&level=response" -O ge_some.xml

~/seiscomp3/bin/seiscomp exec fdsnxml2inv --debug ge_some.xml | xmllint --format - > ge_inv.xml

cat >ge_mod.rules <<EOF
Nw: GE 2003/001 
Na: Description="A better network description than before"
Na: Archive=PLEDC
EOF

grep -A 5 "<network" ge_inv.xml

~/seiscomp3/bin/seiscomp exec tabinvmodifier -r ge_mod.rules --inventory-db ge_inv.xml -o foo.xml

grep -A 6 "<network" foo.xml 

}

# ----------------------------------------------------------------------

test_usage () {
	echo "test_usage"

	cat >expected <<EOF
No rule file was supplied for processing
Try --help for help
EOF
	~/seiscomp3/bin/seiscomp exec tabinvmodifier >1 2>&1
	diff -q expected 1
	if [ $? -ne 0 ] ; then
		diff -s expected 1
		exit 1
	fi
	echo " - Test no rule file message"
	rm expected 1


	cat >expected <<EOF
Cannot send notifiers when loading inventory from file.
Try --help for help
EOF
	~/seiscomp3/bin/seiscomp exec tabinvmodifier -r rules1 --inventory-db 4a_inv.xml >1 2>&1
	diff expected 1
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	echo " - Test no output file message"
	rm expected 1

	~/seiscomp3/bin/seiscomp exec tabinvmodifier -r rules0 >1 2>&1
	if [ $? -ne 0 ] ; then
		cat 1
        	exit 1
	fi
	echo " - Test empty rules file"
}

# ---
test_nosuchrulesfile () {
	echo "test_nosuchrulesfile"

	f=nosuchrulesfile.rules
	cat >expected <<EOF
tabinvmodifier: $f: No such file or directory
Try --help for help
EOF
	if [ -f "$f" ] ; then echo "Oops" && exit 1 ; fi

	~/seiscomp3/bin/seiscomp exec tabinvmodifier -r $f >1 2>&1
	diff expected 1
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	echo "Test non-existent rules file"
}

# ----------------------------------------------------------------------

test2 () {
# Wrong case on attribute names. Based on Valentino L.'s example.
	echo "test2"

	echo "Expect key error..."
	~/seiscomp3/bin/seiscomp exec tabinvmodifier -r rules1

# First version (Nov 2016) threw KeyError:
#
#   File "/home/pevans/seiscomp3/bin/tabinvmodifier", line 263, in _modifyInventory
#      p = valid['attributes'][k]['validator'](p)
#   KeyError: 'archive'

	echo "Expect write to DB..."
	~/seiscomp3/bin/seiscomp exec tabinvmodifier -r rules2 --debug
}

test_offline () {
# Expect success - set netClass and archive on <network>, and archive on <stations>.
	echo "test 3"
	~/seiscomp3/bin/seiscomp exec tabinvmodifier -r rules2 --inventory-db 4a_inv.xml -o 4a_new.xml
	diff 4a_inv.xml	4a_new.xml
}

setup
test_nosuchrulesfile
test_usage
test2
test_offline
teardown

