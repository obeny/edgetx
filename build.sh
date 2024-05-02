#!/bin/bash

#patching
CURDIR=$(pwd)
#lvgl
cd radio/src/thirdparty/libopenui/thirdparty/lvgl
cat $(ls ${CURDIR}/patches/radio/src/thirdparty/libopenui/thirdparty/lvgl/*.patch | sort) | patch -p1
#end patching
cd ${CURDIR}

#build
python3 -m venv venv
. venv/bin/activate
pip install -r requirements.txt

mkdir -p build
cd build

export PATH="/opt/toolchains/tc_arm-none-eabi/bin:${PATH}"

../cmake_cmd

set -e
make configure
make -j`nproc` firmware
#make -j`nproc` companion
