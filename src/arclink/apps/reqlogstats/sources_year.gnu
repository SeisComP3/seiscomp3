load 'year_ticks.gnu'
# Override for now, shorter range for early in the year:
jdays = 63
set xrange [0:jdays]

show xtics
set ylabel 'total_size, MiB'
set yrange [0:]

load 'histo_settings.gnu'

if (jdays > 300) { set key top left inside } else { set key top right inside }
set key maxrows 5

set terminal svg font "arial,14" size 960,480 dynamic
set output 'out.svg'

load 'node_colors.gnu'

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
