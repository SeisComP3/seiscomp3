set style data lines

!cat users.out | sed -e 's/|/  /g'     | python t2.py > days3_g.dat
!head -3 days3_g.dat

set xdata time
set timefmt "%Y-%m"
set xtics format "%m"
set title 'Daily distinct user monthly averages in 2016'
n=14
plot '<python monthsums.py <days3_g.dat' using 1:($3/$14) title 'BGR', \
     '' u 1:($4/n) title 'ETHZ', \
     '' u 1:($5/n) title 'GFZ', \
     '' u 1:($6/n) title 'INGV', \
     '' u 1:($7/n) title 'IPGP', \
     '' u 1:($8/n) title 'KOERI', \
     '' u 1:($9/n) title 'LMU', \
     '' u 1:($10/n) title 'NIEP', \
     '' u 1:($11/n) title 'NOA', \
     '' u 1:($12/n) title 'ODC'

