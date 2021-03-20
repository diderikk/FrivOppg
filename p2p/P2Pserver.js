const http = require("http");
const fs = require("fs").promises;
const ws = require("./WebSocket.js");

const HTTPPORT = 3000;

ws.startWebSocket();

const httpServer = http.createServer((req, res) => {
  if (req.method === "GET") {
    if (req.url === "/index.js") {
      fs.readFile(__dirname + "/frontend/index.js")
        .then((contents) => {
          res.setHeader("Content-Type", "application/javascript");
          res.writeHead(200);
          res.end(contents);
        })
        .catch((err) => {
          res.writeHead(500);
          res.end(err);
          return;
        });
    } else if (req.url === "/style.css") {
      fs.readFile(__dirname + "/frontend/style.css")
        .then((contents) => {
          res.setHeader("Content-Type", "text/css");
          res.writeHead(200);
          res.end(contents);
        })
        .catch((err) => {
          res.writeHead(500);
          res.end(err);
          return;
        });
    } else {
      fs.readFile(__dirname + "/frontend/index.html")
        .then((contents) => {
          res.setHeader("Content-Type", "text/html");
          res.writeHead(200);
          res.end(contents);
        })
        .catch((err) => {
          res.writeHead(500);
          res.end(err);
          return;
        });
    }
  }
});

httpServer.listen(HTTPPORT, () => {
  console.log("Server listening on port", HTTPPORT);
});

httpServer.on("error", (error) => {
  console.log(error);
});
