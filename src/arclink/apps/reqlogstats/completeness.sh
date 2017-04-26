
set -u

progname=`basename $0`


show_usage() {
	echo "${progname} [ {month} [ {db file} ]]"
	echo
	echo "Create a summary of reports for a month." 
	echo "If {month} is not given, use last month."
	echo "If {db file} is not given, use a default."

}

if [ $# -gt 0 ] ; then
    start_month=$1
    shift
else
    start_month=$(date +%m -d "last month")
fi

if [ $# -gt 0 ] ; then
    db_file=$1
    start_year=$(echo $1 | sed -e 's/[^0-9]//g')
    db_dir=$(dirname $1)
    shift
else
    start_year=$(date +%Y)
    db_dir="var"
    db_file="${db_dir}/reqlogstats-${start_year}.db"
fi
echo "Looking in ${db_file} for month ${start_month} of ${start_year}" 

if [ $# -gt 1 ]; then
    show_usage
    exit 1
fi

dcid_list=$(echo "SELECT distinct(dcid) FROM ArcStatsSource ORDER BY dcid;" | sqlite3 var/reqlogstats-2016.db)

for dcid in $dcid_list ; do
    echo -n "$dcid : "
    cmd="SELECT count(start_day) FROM ArcStatsSummary AS X JOIN ArcStatsSource as Y WHERE X.src = Y.id AND Y.dcid='${dcid}';"
    echo ${cmd} \
	| sqlite3 ${db_file}
done
