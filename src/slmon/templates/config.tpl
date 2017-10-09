;; Generated at $date - Do not edit
;; template: $template

[setup]

;; Web title of the SeedLink Monitor
title       = $title

;; Email to appear on the web page
email       = $email

;; Mounted directory into which the .html files will be written.
;; This is ESSENTIAL!!!
wwwdir      = $wwwdir

;; Shortcut icon for the web page. Noting really important;
;; if in doubt, just comment it out.
icon        = $icon 

;; Name string of Link on footer line
;; if in doubt, just comment it out.
linkname     = $linkname

;; Link URL on footer line
;; if in doubt, just comment it out.
linkurl     = $linkurl

;; Specify URL of GEOFON-style "live seismograms". The '%s'
;; will be replaced by the station name. If no live seismograms
;; are available or desired, comment this entry out.
liveurl    = $liveurl

;; refresh rate in seconds
refresh	    = $refresh

;; SeedLink server - ESSENTIAL!!!
server      = $address:$port

[colors]
; define colors depending on the latency
; NOT CURRENTLY USED!!!!
color0 =      0:#cc99ff
color1 =   1800:#3399ff
color2 =   3600:#00ff00
color3 =   7200:#ffff00
color4 =  21600:#ff9966
color5 =  86400:#ff3333
color6 = 172800:#ffcccc
color7 = 259200:#cccccc
color8 = 345600:#999999
color9 = 432000:#666666
