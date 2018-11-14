load 'year-ticks.gnu'
# Override for now, shorter range for early in the year:
jdays = 63
set xrange [0:jdays]

# Trickery to make level 0 ticks smaller than level 1, and without text labels
##set xtics 0,7,366 format "" scale 0.5,1

show xtics

set ylabel 'Size by day for type "#volume_type_patt#" (MiBytes)'

if (jdays > 300) { set key top left inside } else { set key top right inside }

set key vertical maxrows 6
set key title "Data served by DCID"
set key spacing 1.25  # vertical space
set key width 0.25
set key box

load 'histo_settings.gnu'

set terminal svg font "arial,14" size 960,480 dynamic
set output 'out.svg'

load 'node_colors.gnu'

#mib(x) = (x/1024.0/1024.0)

###plot '<cut -c9- days3.dat' using 3:xtic(int(\$0) % ${xtic_density} == 0?sprintf("%i", \$0):"") title 'BGR' ls 2, 
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
     '' using 13 title 'RESIF' ls 8, \
     '' using 2 title 'All' with lines ls -1

#set terminal dumb
#set output
#replot
