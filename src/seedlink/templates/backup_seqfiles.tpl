#!/bin/bash

# Generated at $date - Do not edit!
# template: $template

KEEP=30

mkdir -p "$pkgroot/var/lib/seedlink/backup"

find "$pkgroot/var/lib/seedlink/backup" -type f -mtime +$$KEEP -exec rm -f '{}' \;

cd "$pkgroot/var/run/seedlink"
tar cf - *.seq *.state | gzip >"$pkgroot/var/lib/seedlink/backup/seq-"`date +'%Y%m%d'`.tgz
