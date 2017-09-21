set xdata time
set timefmt "%Y-%m-%d"
set xlabel 'Day in #year#'
set xrange ['#year#-01-01':]
set xtics #xtic_density# * 24 * 3600
set xtics format "%j"
set ylabel 'total_size, MiB'
set logscale y

set key top left
set grid x
set style data linespoints

set terminal svg font "arial,14" size 960,480   # even "giant" is not enough font.
set output 'out.svg'

plot 'days3.dat' using 1:2 title 'All EIDA nodes'

#set terminal dumb
#set output
#replot
