#!/bin/bash

# Generated at $date - Do not edit!
# template: $template

CFGDIR="$slarchive._config_dir"
ARCHIVE="$archive"

for rc in `find $$CFGDIR -name "rc_*"`; do
    station=$${rc##*_}
    source $$rc
    find "$$ARCHIVE"/*/"$$NET/$$STATION" -type f -follow -mtime +$$ARCH_KEEP -exec rm -f '{}' \;
done
