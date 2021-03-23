FROM ubuntu:20.04

RUN apt-get update

RUN apt-get install -y g++

COPY . ./test

COPY ../stunserver ./stunserver

RUN g++ attribute_test.cpp  -o test

RUN ./test

RUN g++ handler_test.cpp  -o test

RUN ./test
