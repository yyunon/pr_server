 # output as png image
 set terminal png

 # save file to "benchmark.png"
 set output "benchmark.png"

 # graph a title
 set title "ab -n 100 -c 10 -g foo.tsv http://foo/"

 # nicer aspect ratio for image size
 set size 1,0.7

 # y-axis grid
 set grid y

 # x-axis label
 set xlabel "request"

 # y-axis label
 set ylabel "response time (ms)"

 # plot data from "foo.tsv" using column 9 with smooth sbezier lines
 plot "foo.tsv" using 9 smooth sbezier with lines title "server1:"
