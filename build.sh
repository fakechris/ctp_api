#!/bin/sh

protoc --cpp_out=./proto_cpp ./proto/thostmd.proto

g++ -g -o test -I . api/trade/linux64/lib/thostmduserapi.so api/trade/linux64/lib/thosttraderapi.so test.cpp md/mdspi.cpp order/traderspi.cpp  proto_cpp/thostmd.pb.cc

