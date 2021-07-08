#!/bin/bash

BUILD_DIR=build
REPORT_DIR=report

cd ../
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR

cmake -DCMAKE_BUILD_TYPE=Debug ..

cmake ../
make -j 16

cd tests/

./dde_dock_unit_test --gtest_output=xml:dde_test_report_dde_dock.xml
lcov -c -d ./ -o cover.info
lcov -e cover.info '*/frame/*' '*/dde-dock/widgets/*' -o code.info
lcov -r code.info '*/dbus/*' '*/xcb/*' -o final.info

rm -rf ../../tests/$REPORT_DIR
mkdir -p ../../tests/$REPORT_DIR
genhtml -o ../../tests/$REPORT_DIR final.info

mv asan.log* asan_dde-dock.log
