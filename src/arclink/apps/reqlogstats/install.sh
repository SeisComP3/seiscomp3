#!/bin/sh
#
# reqlogstats EIDA log statistics
#  - back end and front end
#
#
# To run on eida.gfz-potsdam.de (webdc) at
# http://eida.gfz-potsdam.de/eida/reqlogstats/
#
# There are two components:
# 1. Accumulator ("back end"), processes reports from e-mail
# 2. Report generator ("front end"), runs on web server.
#
#
# Item 1 requires a crontab entry to run
#  reqlogstats.sh once per day.
#  Its code and database should not go under ServerRoot.
#
#
# Item 2 requires some additional style sheets at
# http://eida.gfz-potsdam.de/css
#
# basic.css, gfz-backup.css
#
# from svn+ssh://st32/srv/svn/repos/geofon/trunk/www/css
#
# e.g. sysop@geofon-open2:/srv/www/webdc/
# To install these by hand, onto webdc:
#  cd /srv/www/webdc
#  svn checkout svn+ssh://localhost/srv/svn/repos/geofon/trunk/www/css

set -u

show_usage() {
  echo "Usage: ${progname} {target}"
  echo "Install the reqlogstats web server files on host {target}"
  echo " e.g. geofon-open1 or geofon-open2"
}

progname=`basename $0`

install_accumulator() {
	diffs_found=0
	mkdir -p tmp
	for f in * ; do
		if [ ! -f $f ] || [ "$f" = "install.sh" ]; then
			continue
		fi
	        scp $target:$progdir/$f tmp 
		diff -q $f tmp
	        if [ $? -ne 0 ] ; then
	                echo "diff $f tmp"
		        diffs_found=1
	        fi
	done

	if [ $diffs_found -eq 0 ] ; then
	        echo "Identical code at $target:$progdir; nothing to update"
	        rm -r tmp
	else
		echo "Starting rsync to update code at $target:$progdir in $rsync_delay seconds..."
	        echo "CTRL-C to stop it."
	        sleep $rsync_delay
		# Non-recursive rsync:
		rsync -v * --exclude install.sh --exclude tmp --exclude var --exclude webdc --exclude www $target:$progdir
		ssh $target "(cd $progdir ; mkdir -p var )"
	fi

}


if [ $# -ne 1 ] ; then
	show_usage
	exit 2
fi

rsync_delay=10 # Seconds to wait before starting a dangerous rsync.
target=$1
if [ $target = "geofon-open1" ] || [ $target = "geofon-open2" ] ; then
	remotedir=/srv/www/webdc/eida/reqlogstats
	progdir=/home/sysop/reqlogstats
	webdcdir=${target}:${remotedir}
else
	echo "Giving up, no target given."
	echo
	show_usage
	exit 1
fi

git status

install_accumulator

diffs_found=0
pushd www
mkdir -p webdc
for f in * ; do
	scp $webdcdir/$f webdc
	diff -q $f webdc
	if [ $? -ne 0 ] ; then
		echo "diff www/$f www/webdc"
		diffs_found=1
	fi
done

if [ $diffs_found -eq 0 ] ; then
	echo "Identical code at $webdcdir; nothing to update"
	rm -r webdc
else
	echo "Starting rsync to update code at $webdcdir in $rsync_delay seconds..."
	echo "CTRL-C to stop it."
	sleep $rsync_delay
	rsync -av * --exclude webdc $webdcdir
fi
popd

echo "Testing code at ${target}..."

echo "Expect 0:"
ssh ${target} "(cd ${remotedir}; php reqlog.php | wc -c)"

for f in reqlogdisplay.php reqlognetwork.php ; do
	echo "Expect \"/dev/stdin: XML\""
	ssh ${target} "(cd ${remotedir}; php $f | file - )"
done

