#!/bin/sh
rm -rf __install
rm bin/*
rm libs/*
make clean
make
make install
