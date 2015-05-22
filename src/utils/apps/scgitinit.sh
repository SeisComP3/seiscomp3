#!/bin/bash
# Initializes a GIT repository in $SEISCOMP_ROOT and adds important
# configuration files from 'etc' and 'share' directory
#
# Author: Stephan Herrnkind <herrnkind@gempa.de>


# search for SeisComP path
if [ x"$SEISCOMP_ROOT" = x ]; then
	echo "SEISCOMP_ROOT not set"
	exit 1
fi

# search git binary
which git > /dev/null
if [ $? -ne 0 ]; then
	echo "git binary not found"
	exit 2
fi

cd $SEISCOMP_ROOT || exit 3

# initialize git if necessary
[ -d .git ] || git rev-parse --git-dir > /dev/null 2>&1
if [ $? -eq 0 ]; then
	echo "GIT repository in $SEISCOMP_ROOT already initialized"
else
	git init || exit 4
fi

# add files
git add etc
find share -type f -regex \
     ".*\.\(bna\|cfg\|conf\|htaccess\|kml\|py\|sh\|tpl\|tvel\|txt\|xml\)" \
     -execdir git add {} +

echo "files added to GIT, use 'git status' to get an overview and " \
     "'git commit' to commit them"
