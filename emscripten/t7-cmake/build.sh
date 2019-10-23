#!/bin/bash

if [ ! -d "tmp" ];
then 
	mkdir tmp
fi

cd tmp
rm -rf ./*

emconfigure cmake ..
emmake make