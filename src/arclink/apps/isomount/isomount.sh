#!/bin/bash

#####################################################################
# isomount.sh                                                       #
#                                                                   #  
# Deals with the (un)mounting procedure for a given ISO image file. # 
#                                                                   #
# Exit status: -1 means locked                                      #
#                 prints error message on stderr                    #
#               0 means ok                                          #
#                 prints the mount point on stdout                  #
#               1 means any other error or signal                   #
#                 prints error message on stderr                    #
#                                                                   #
# @date:        2006-07-26                                          #
# @author:      Doreen Pahlke                                       #
# @institution: GFZ Potsdam                                         #
#                                                                   #
#####################################################################

### constants and global flags ###
LOCKFILE=$SEISCOMP_ROOT/arclink/status/.lock__
REFFILE=$SEISCOMP_ROOT/arclink/status/.references__
MAPFILE=$SEISCOMP_ROOT/arclink/status/.mappings__
MOUNTED=0   # flag which ensures a consistent roll back after a done mount
UNMOUNTED=0 # flag which ensures a consistent roll back after a done umount

###
### function help
### prints the help message for script usage
### @arguments: None
###
help() {
	cat <<HELP

-----------------------------------------------------------------------------------

 Deals with the (un)mounting procedure for a given ISO image file.

 Exit status: -1 means locked
                 prints error message on stderr
               0 means ok
                 prints the mount point on stdout
               1 means any other error or signal
                 prints error message on stderr
 
-----------------------------------------------------------------------------------

USAGE:   isomount.sh [-h] -f <fname> -m | -u

OPTIONS: -h show this help information
         -f <fname> (mandatory option !) defines the full path to an ISO image file
         -m (alternative mandatory option !) means mount the given file
         -u (alternative mandatory option !) means unmount the given file

HELP
	exit 1
}

###
### function clean
### removes temporary and lock files
### @arguments: None
### @return: None
###
clean() {
		rm -f "${REFFILE}~"
		rm -f "${MAPFILE}~"
		rm -f "$LOCKFILE"
}

###
### function unlock_and_roll_back
### remove lock file and roll back
### @arguments: None
### @return: None
###
unlock_and_roll_back() {
		rm -f "$LOCKFILE"
		test -e "${REFFILE}~" && ( mv "${REFFILE}~" "$REFFILE" || error_exit "Error: reseting the reference count file failed!" )
		test -e "${MAPFILE}~" && ( mv "${MAPFILE}~" "${MAPFILE}" || error_exit "Error: reseting the mapping file failed!" )
		test $MOUNTED -eq 1 && umount $mountpoint
		test $UNMOUNTED -eq 1 && mount $mountpoint
}

###
### function error_exit
### displays the given error message
### @arguments: a string specifying the error message
### @return: None
###
error_exit() {
		echo -n "${0}: ${1} Exit." >&2
		unlock_and_roll_back
		exit 1
}

###
### function sig_exit
### performs exit if signal is trapped
### @arguments: None
### @return: None
###
sig_exit() {
		echo -n "${0}: Trapped a signal. Exit." >&2
		unlock_and_roll_back
		exit 1
}

###
### function check_arg
### checks the user given input
### @arguments: a string giving a file name
### @return: 0 if the given file name exists; 1 otherwise
###
check_arg() {
		if [ -e "$1" ]; then
				return 0
		else
				return 1
		fi
}

###
### function init
### initialising: copy the file which stores the mapping of mount points and references to them
### @arguments: None
### @return: None
###
init() {
		test -e "$REFFILE" || ( touch "$REFFILE" || error_exit "Error: creating the reference count file failed!" )
		cp "${REFFILE}" "${REFFILE}~" || error_exit "Error: copying the reference count file failed!"
		test -e "${MAPFILE}" || ( touch "${MAPFILE}" || error_exit "Error: creating the mapping file failed!" )
		cp "${MAPFILE}" "${MAPFILE}~" || error_exit "Error: copying the mapping file failed!"
}

###
### function is_mounted
### checks for a given mount point whether it is already mounted
### @arguments: a string defining a mount point
### @return: 0 if the given mount point is currently mounted; 1 otherwise
###
is_mounted() {
		local teststr
		teststr=`cat $MAPFILE | grep $1`
		
		if [ -n "$teststr" ]; then
				return 0
		else
				return 1
		fi
}

###
### function is_element
### checks whether a given file name is element of a set of file names
### @arguments: 1) a file name which schould be tested; 2) a string giving the set of file names
### @return: 0 if the given file name is element of the given set; 1 otherwise
###
is_element() {
		local item

		for item in $2; do
				if [ `basename $item` == `basename $1` ]; then
						return 0
				fi
		done

		return 1
}

###
### function check_mount_points
### checks the system environment for ISO9660 loop device mount points
### @arguments: None
### @return: None
###
check_mount_points() {
		isolinks=`cat /etc/fstab | grep iso9660 | awk '{ printf("%s ",$1) }'`
		isolinks=${isolinks% }
		mountpoints=`cat /etc/fstab | grep iso9660 | awk '{ printf("%s ",$2) }'`
		mountpoints=${mountpoints% }

		test -z "$isolinks" -o -z "$mountpoints" && error_exit "Error: no iso9660 loop device found on the system!"
}

###
### function get_mount_status
### checks which devices are currently mounted
### @arguments: None
### @return: 0 if there are ISO images mounted; 1 otherwise
###
get_mount_status() {
 		isostatus=`cat $MAPFILE | awk '{ printf("%s ",$1) }'`

 		if [ -z "$isostatus" ]; then
				return 1
 		else
 				return 0
 		fi
}

###
### function save_mapping
### stores the mapping of ISO image file to its mount point
### @arguments: 1) a string specifying the file name; 2) a string giving the mount point
### @return: 0 if the mapping is stored; 1 if there already exists a mapping
###
save_mapping() {
		local fname
		local mp
		local setflag=0
		local newmap=""
		
		while read fname mp; do
				newmap="${newmap}$fname $mp\n"
				if [ "$1" == "$fname" ]; then
						if [ "$2" == "$mp" ]; then
								setflag=1
						else
								error_exit "Error: there already exists a mapping for $fname to $mp instead of $2!"
						fi
				fi
		done < <(cat ${MAPFILE}~)
		
		if [ $setflag -eq 0 ]; then
				newmap="${newmap}$1 $2\n"
		fi
		printf "$newmap" > $MAPFILE || error_exit "Error: writing the mapping file failed!"
			
		return $setflag
}

###
### function remove_mapping
### removes the mapping of ISO image file to its mount point
### @arguments: a string specifying the file name
### @return: 0 if the mapping is stored; 1 otherwise
###
remove_mapping() {
		local fname
		local mp
		local setflag=0
		local newmap=""

		while read fname mp; do
				if [ "$1" != "$fname" ]; then
						newmap="${newmap}$fname ${mp}\n"
				else
						setflag=1
				fi
		done < <(cat ${MAPFILE}~)
		printf "$newmap" > $MAPFILE || error_exit "Error: writing the mapping file failed!"

		return $setflag
}

###
### function increase_reference
### increases the reference counter in the file storing the mapping of mount points and references on them
### @arguments: a string specifying a mount point
### @return: 0 if the reference counter is new and set to 1; 1 otherwise
###
increase_reference() {
		local mp
		local ref
		local setflag=0
		local newmap=""

		while read mp ref; do 
				if [ "$1" == "$mp" ]; then
						newmap="${newmap}${mp} $(( $ref+1 ))\n"
						setflag=1
				else
						newmap="${newmap}${mp} $ref\n"
				fi
		done < <(cat ${REFFILE}~)

		test $setflag -eq 0 && newmap="${newmap}${1} 1\n"
		printf "$newmap" > $REFFILE || error_exit "Error: writing the reference count file failed!"

		return $setflag
}

###
### function decrease_reference
### decreases the reference counter in the file storing the mapping of mount points and references on them
### @arguments: a string specifying a mount point
### @return: 0 if the reference counter equals 0; 1 otherwise
###
decrease_reference() {
		local mp
		local ref
		local retval=0
		local setflag=0
		local newmap=""

		while read mp ref; do 
				if [ "$1" == "$mp" ]; then
						setflag=1
						if [ $ref -gt 1 ]; then
								newmap="${newmap}${mp} $(( $ref-1 ))\n"
								retval=1
						fi						
				else
						newmap="${newmap}${mp} $ref\n"
				fi
		done < <(cat ${REFFILE}~)

		test $setflag -eq 0 && error_exit "Error: given mount point ${1} not found in the reference count file!"
		printf "$newmap" > $REFFILE || error_exit "Error: writing the reference count file failed!"

		return $retval
}

###
### function mount_iso
### mounts the given ISO image file
### @arguments: a string specifying the ISO image file name
### @return: None
###
mount_iso() {
	  mountpoint=""
		local lname=""
		local base=`basename $1`

		if ( get_mount_status && is_element "$1" "$isostatus" ); then
				 mountpoint=`cat $MAPFILE | grep $base | awk '{ printf $2 }'`
				 lname=`cat /etc/fstab | grep $mountpoint | awk '{ printf $1 }'` 
		fi

		if [ -z "$mountpoint" ]; then
				check_mount_points		
				for mountpoint in $mountpoints; do
						is_mounted "$mountpoint" || {
								lname=`cat /etc/fstab | grep $mountpoint | awk '{ printf $1 }'`
								if [ -z "$lname" ]; then
										error_exit "Error: extraction of the device name for mount point $mountpoint from fstab failed!"
								fi
								rm -f "$lname"
								ln -s "$1" "$lname"
								break
						}
				done
		fi

		if [ -z "$lname" ]; then
				error_exit "Error: All loop devices in use!"
		else
				if ( increase_reference "$mountpoint" && ( `mount "$mountpoint"` || error_exit "Error: mount $mountpoint as mount point of $1 failed!" ) ); then
						MOUNTED=1
						save_mapping $base $mountpoint
				fi
		fi

		echo -n "$mountpoint" >&1
}

###
### function umount_iso
### unmounts the given ISO image file name
### @arguments: a string specifying the ISO image file name
### @return: None
### 
umount_iso() {
		local base=`basename $1`
		mountpoint=`cat $MAPFILE | grep $base | awk '{ printf $2 }'`

		if [ -n "$mountpoint" ]; then
				if ( decrease_reference "$mountpoint" && ( `umount "$mountpoint"` || error_exit "Error: umount $mountpoint as mount point of $1 failed!" ) ); then
						UNMOUNTED=1
						remove_mapping $base $mountpoint
				fi
		else
				error_exit "Error: tried to umount but there does not exist a mount point for ${1}!"
		fi

		echo -n "$mountpoint" >&1
}

###
### function is_stale_lock
### checks whether the lock file is stale
### @arguments:
### @return: 0 if the lock file is stale; 1 otherwise
###
is_stale_lock() {
		local pid=`cat "$LOCKFILE"`
		local curpids=`pgrep isomount.sh`
		local findres=`find $LOCKFILE -mmin +3`
		
		is_element "$pid" "$curpids" || test -z "$findres" || return 0
		return 1
}
################################### MAIN ##########################################
m_opt=0
u_opt=0

### parse the command line options ###
while getopts ":hf:mu" opt; do
		case "$opt" in
				h ) help
						exit 0;;
				f ) fname=$OPTARG
						check_arg "$fname" || error_exit "Error: file $fname could not be found!";;
				m ) m_opt=1
						action="mount_iso";;
				u ) u_opt=1
						action="umount_iso";;
				* ) help
						exit 1
		esac
done
# if there is no option given show help string
test "$OPTIND" == "1" && help

### check the user given options ###
test -z "$fname" && error_exit "Error: Missing mandatory option -f <fname>! Type -h for help!"
test $m_opt -eq 0 -a $u_opt -eq 0 && error_exit "Error: Missing mandatory option! Type -h for help!"
test $m_opt -eq 1 -a $u_opt -eq 1 && error_exit "Error: The options -m and -u are alternatives! Please choose one of both only!"

### test and write lockfile to avoid a race condition during mount or umount ###
# by setting the noclobber option the redirection is an atomic test and set operation for file locking #
if ( set -C; echo "$$" > "$LOCKFILE" ) 2> /dev/null; then
    # in case of certain signal trap call signal handler #
		trap sig_exit ABRT FPE HUP ILL INT KILL QUIT SEGV STOP SYS TERM
		init
	  # critical section #
		"$action" "$fname"
    ####################
		clean
		trap - ABRT FPE HUP ILL INT KILL QUIT SEGV STOP SYS TERM
		exit 0
else
		echo -n "${0}: Failed to acquire lockfile: ${LOCKFILE}. Lock is owned by process `cat ${LOCKFILE}`" >&2
#		is_stale_lock && echo "${0}: !!! THE LOCKFILE SEEMS TO BE STALE !!!" >&2
		exit -1
fi 



