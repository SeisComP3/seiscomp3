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

