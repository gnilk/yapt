#!/bin/sh
./yapt2
python dochart.py --format="pdf" > plot.pdf
open plot.pdf 


