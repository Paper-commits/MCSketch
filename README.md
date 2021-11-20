# MCSketch
## Introduction
we present MCSketch, a novel sketch for heavy flow detection. It adopt power-weakening increment method which allow sufficient competition in the early stage of identifing heavy flow and amplify relative advantage when the counter is larg enough.

All algorithms in this repo are implemented in C++.
## About this repo
data: we have solved the CAIDA data extracting the SrcIP as fingerprint in our experiment.We provide one data for testing.
demo: main
common: basic funtion
## Requirements
- g++ >= 5.4
- make >= 4.1
we have test this repository in ubuntu 16.04
## How to run
you can build these algorithms by
```
$ cd demo
$ make clean
$ make
$ ./main.out
```
Next, you can input data, and different memory and threshold for testing.
