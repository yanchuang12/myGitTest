#!/bin/bash

gnuplot -persit <<-EOFMarker
  plot "b.plt" w lp t "b"
  replot "c.plt" w lp t "c"
EOFMarker
