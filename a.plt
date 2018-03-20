set xlabel "months"
set ylabel "preciptation in mm"
set title "the urban rainfall"
set xrange [0.5:12.5]
set xtics 1,1,12
plot "jiangshui.dat" u 1:2 w lp pt 1 t "chongqing", "jiangshui.dat" u 1:3 w lp pt 3 t "chengdu", "jiangshui.dat" u 1:4 w lp pt 5 t "fuzhou"

