# docker run -td -p 8080:8080/udp --rm <docker-image>

FROM ubuntu:20.04

RUN apt-get update

RUN apt-get install -y g++

EXPOSE 8080/udp

COPY server.cpp .

COPY server.hpp .

RUN g++ server.cpp -o server

CMD ["./server"]
