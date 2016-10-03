#!/bin/bash

ROOT=$1
PARSER=$2

for f in `find ${ROOT} -name "urls"`; 
do
    cat ${f} | python ${PARSER}
done
