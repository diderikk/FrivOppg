FROM ubuntu:latest

RUN apt-get update

RUN apt install -y nodejs

EXPOSE 3000 3001

COPY . .

CMD ["node","P2Pserver.js"]