const net = require('net');
const crypto = require("crypto");
const http = require('http');
const fs = require('fs').promises;
const ws = require('./WebSocket.js')

const WSPORT = 3001;
const HTTPPORT = 3000;
let clients = [];


let firstOffer = null;

ws.test();

const httpServer = http.createServer((req, res) => {
    // console.log(req);
    // 
    if (req.method === 'GET') {
        if (req.url === '/index.js') {
            fs.readFile(__dirname + '/index.js')
                .then(contents => {
                    res.setHeader("Content-Type", "application/javascript");
                    res.writeHead(200);
                    res.end(contents);
                })
                .catch(err => {
                    res.writeHead(500);
                    res.end(err);
                    return;
                });
        }
        else {
            fs.readFile(__dirname + '/index.html')
                .then(contents => {
                    res.setHeader("Content-Type", "text/html");
                    res.writeHead(200);
                    res.end(contents);
                })
                .catch(err => {
                    res.writeHead(500);
                    res.end(err);
                    return;
                });
        }
    }
    else if (req.method === 'POST') {
        if (req.url === '/api/v1/create') {
            let responseBody = '';
            req.on("data", (data) => {
                responseBody += data;
            });
            req.on('end', () => {
                firstOffer = JSON.parse(responseBody);
                console.log(firstOffer);
                res.writeHead(200);
            });
        }
        else if (req.url == '/api/v1/connect') {
            res.writeHead(200);
            res.setHeader("Content-Type", "application/json");
            res.end(JSON.stringify(firstOffer));
        }
    }
});

httpServer.listen(HTTPPORT, () => {
    console.log('Server listening on port', HTTPPORT);
})

httpServer.on("error", (error) => {
    console.log(error);
});