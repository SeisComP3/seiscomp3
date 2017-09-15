set xdata time
set key top left
set timefmt "%Y-%m-%d"
set style data linespoints
plot '<sed -e "s/|/ /g" reqlogstats_volume_difference-2017.dat' using 1:2 title 'Volume "fdsnws"', '' u 1:3 title 'User "fdsnws@geofon"', '' u 1:4 title "V - U"

