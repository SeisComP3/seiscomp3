set xlabel 'Date in #year#'
set xrange [0:366]
set xtics nomirror out
# Trickery to make level 0 ticks smaller than level 1, and without text labels
##set xtics 0,7,366 format "" scale 0.5,1
set xtics 0,7,366 format "" 
set mxtics 7 
set xtics add ("Jan" 1, "Feb" 32, "Mar" 60, "Apr" 91, "May" 121, "Jun" 152, "Jul" 182, "Aug" 213, "Sep" 244, "Oct" 274, "Nov" 305, "Dec" 335)
show xtics

set ylabel 'Size by day for type "#volume_type_patt#" (MiBytes)'

set key top left inside
set key vertical maxrows 4
set key title "Data served by DCID"
set key spacing 1.25  # vertical space
set key width 0.25
set key invert        # For stacked histogram
set key box
set nogrid

set style data histograms
set style histogram rowstacked
set boxwidth 1.0 relative
set style fill solid 0.9 noborder

set terminal svg font "arial,14" size 960,480 dynamic
set output 'out.svg'

# Default for ls 6 is dark blue, but that is too close to pure blue for GFZ:
set style line 3 linecolor rgb "#00589C"
set style line 5 linecolor rgb "skyblue"
set style line 6 linecolor rgb "violet"
set style line 10 linecolor rgb "magenta"

#mib(x) = (x/1024.0/1024.0)

###plot '<cut -c9- days3.dat' using 3:xtic(int(\$0) % ${xtic_density} == 0?sprintf("%i", \$0):"") title 'BGR' ls 2, 
plot '<cut -c9- days3.dat' using 3 title 'BGR' ls 2, \
     '' using  4 title 'ETHZ' ls 1, \
     '' using  5 title 'GFZ' ls 3, \
     '' using  6 title 'INGV' ls 4, \
     '' using  7 title 'IPGP' ls 6, \
     '' using  8 title 'KOERI' ls 1, \
     '' using  9 title 'LMU' ls 7, \
     '' using 10 title 'NIEP' ls 10, \
     '' using 11 title 'NOA' ls 5, \
     '' using 12 title 'ODC' ls 9, \
     '' using 13 title 'RESIF' ls 8, \
     '' using 2 title 'All' with lines ls -1

#set terminal dumb
#set output
#replot
