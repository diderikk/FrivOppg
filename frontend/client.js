const net = require('net');
const fs = require('fs');

// Simple HTTP server responds with a simple WebSocket client test
const httpServer = net.createServer((connection) => {
    connection.on('data', () => {
        try {
            fs.readFile('index.html', (err, data) => {
                if (err) {
                    console.log("Error during reading");
                }
                connection.write('HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ' + data.length + '\r\n\r\n' + data);

            });
        } catch (error) {
            console.log("Couldn't open HTML file");
        }

    });
});


httpServer.listen(3001, () => {
    console.log('HTTP server listening on port 3001');
});

httpServer.on("error", (error) => {
    console.log(error);
});