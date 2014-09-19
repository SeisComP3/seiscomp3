#!/bin/bash

# Updates, compiles and installs seiscomp3
# Returns 0 on success, 
#         -1 commandline parsing failed
#         -2 if update failed
#         -3 if make failed
#         -4 if intallation failed
#         -5 seiscomp start|stop failed (console flag)

usage="Usage: $0 -t [gui(not supportedt yet), console]"


# Parsing error or no flags
#if [ $# -le 3 ]; then
#  echo $usage
#  exit -1
#fi

type=''

while [ $# -ge 1 ]; do
  case $1 in
    -t ) 
    	shift; 
	if [ $# -eq 0 ]; then echo $usage; exit -1; fi
	type="$1";;

	-h )
		echo $usage; exit -1;;
		
    * ) echo "ERROR: unknown flag $1"; echo $usage; exit -1;;
  esac
  shift
done


if [ $type ]; then
    if [ $type != "gui" -a $type != "console" ]; then
        echo 'Wrong parameter name!'
	echo $usage
	exit -1
    fi
fi	


# Update trunk
test -x update && ./update

if [ $? -ne 0 ]; then exit -2; fi

# Change to build directory
cd build

# Build source
make

if [ $? -ne 0 ]; then exit -3; fi

make install

if [ $? -ne 0 ]; then exit -4; fi

cd ..

if [ $type ]; then
    if [ $type == "console" ]; then
	echo "[ Stopping Seiscomp ]"
	seiscomp stop
	echo "[ Starting Seisccomp ]"
	seiscomp start
	echo "[ Checking if seiscomp is running ]"
	seiscomp check

        if [ $? -ne 0 ]; then exit -5; fi
    fi	
fi

#if [ $type == "gui" ]; then
#	killall scmapview
#	killall sceventsummaryview
#	killall scoriginlocatorview
#	killall sctraceview
#
#	smapview &
#	sceventsummaryview &
#	scoriginlocatorview &
#	sctraceview &
#fi

exit 0


