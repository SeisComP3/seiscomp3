#!/bin/sh
#
# Specific to GEOFON (geofon-open*); could be merged into reqlogstats.sh?
#
# Begun by Peter L. Evans, 2013-2014
# <pevans@gfz-potsdam.de>
# 
# Copyright (C) 2013-2016 Helmholtz-Zentrum Potsdam - Deutsches GeoForschungsZentrum GFZ
#
# This software is free software and comes with NO WARRANTY.
#
# ----------------------------------------------------------------------

set -u

start=$(date +%d-%b-%Y -d "10 days ago")
latest=var/recent.mail
rm -f ${latest}
echo "copy (sentsince ${start}) ${latest}" \
    | mailx -n -N -f $HOME/mail/eida_log
python split_mail.py < ${latest}
rm -f ${latest}

# Special hack for GFZ mail, Jan 2014!
#for f in eida_stats/2014/01/msg-sysop_gfz-potsdam_de-2014-* ; do
#   sed -e '/<\/html>/,//d' -e 's/<\/body>/<\/body><\/html>/' $f > tmp ; mv tmp $f ;
#done

