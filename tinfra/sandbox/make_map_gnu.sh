#!/bin/sh
# TODO: insert new copyright notice

victim=$1;

nm -f b -n -l -C ${victim} | grep -i " t " | grep -v ".text" > ${victim}.map
