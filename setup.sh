#!/bin/bash
set -x -o -e
git submodule init
git submodule update

echo 'You should have esp8266 git version installed in ESP_ROOT of Makefile (~/Arduino/esp8266)'
#ln -s esp8266 ~/Arduino/esp8266/
#cd esp8266/tools
#python get.py


