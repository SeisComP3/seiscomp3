set xdata time
set timefmt "%Y-%m-%d"
set xlabel 'Day in #month_name# #year#'
set xrange ['#year#-#month#-01':]
set xtics 24*3600
set xtics format "%d"
set ylabel 'total_size, MiB'
set logscale y
set yrange [0.09:]  ### Not quite right, but tables are rounded to 0.1MB, so this ensures there will always be a finite y-range.

set key top left
set grid
set style data linespoints

set terminal svg font "arial,14" linewidth 2 size 960,480   # even "giant" is not enough font.
set output 'out.svg'

# Round ball markers, black line by default.
plot 'days3.dat' using 1:2 linestyle 7 title 'All EIDA nodes'

#set terminal dumb
#set output
#replot
