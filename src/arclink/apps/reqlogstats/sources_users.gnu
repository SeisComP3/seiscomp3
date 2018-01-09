load 'year_ticks.gnu'
show xtics

set ylabel 'Distinct users'
set yrange [0:350]  # Good for all 11 EIDA nodes, stacked, in 2017.

set key vertical
set key maxrows 3
set key spacing 1.25  # vertical space
set key box

load 'histo_settings.gnu'
set style fill solid 1.0 noborder

set terminal svg font "arial,14" size 960,480 dynamic
set output 'out.svg'

load 'node_colors.gnu'

# Set integer default value for template variable:
#xtic_density = ('#' . 'xtic_density' . '#' ne '#xtic_density#')?value('#xtic_density#'):7
#xtic_density = int(xtic_density)
#print "xtic_density: " . xtic_density
## plot '<cut -c9- days3.dat' using 3:xtic(int($0) % xtic_density == 0?sprintf("%i", $0):"") title 'BGR' ls 2, \
## plot '<cut -c9- days3.dat' using 3:xtic((substr($1, 9, 10) eq '01')?substr($1, 6, 7):"") title 'BGR' ls 2,

plot '<cut -c9- days3.dat' using 3 title 'BGR' ls 2, \
     '' using  4 title 'ETHZ' ls 1, \
     '' using  5 title 'GFZ' ls 3, \
     '' using  6 title 'INGV' ls 4, \
     '' using  7 title 'IPGP' ls 6, \
     '' using  8 title 'KOERI' ls 11, \
     '' using  9 title 'LMU' ls 7, \
     '' using 10 title 'NIEP' ls 10, \
     '' using 11 title 'NOA' ls 5, \
     '' using 12 title 'ODC' ls 9, \
     '' using 13 title 'RESIF' ls 8

#set terminal dumb
#set output
#replot
