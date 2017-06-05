#!/bin/sh

rm -f *.yaml; 
rm -f *~; 
rm -f *.out*;
rm -f pop_diag.*;
rm -f mdrun.*;
rm -f results/LargeLookahead*;
find . -name "RAxML_*hadi*" -print0 | xargs -0 rm
