const net = require('net');
const fs = require('fs');
const crypto = require("crypto");
const clientHandler = require('./ClientHandler');

const WSPORT = 3001;
// let clients = [];


module.exports = {startWebSocket};

function startWebSocket(){
    const wsServer = net.createServer((connection) => {
        console.log("Client connected");

        connection.on('data', (data) => {
            string = data.toString();
            // Checks to see if it trying to connect to the WS server
            if ((/GET \/ HTTP\//i).test(string)) {
                if (!checkHeaderFields(string)) {
                    connection.write("HTTP/1.1 400 Bad Request\r\n");
                    console.log(string);
                    connection.end();
                }
                // Reads key from  client's GET-request
                let key = string.match(/Sec-WebSocket-Key: (.+)?\s/i)[1].toString();
                // Creates acceptKey
                let acceptKey = createAcceptKey(key);

                // Writes answer with acceptKey to client
                connection.write(`HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\n` +
                    `Connection: Upgrade\r\nSec-WebSocket-Accept: ${acceptKey}\r\n\r\n`);
                // Adds client to an array of clients/sockets
                // clients.push(connection);
            }
            else {
                //Checks opcode for connection closing
                if ((data[0] & 0b1111) === 0x8) {
                    connection.end();
                    return;
                };
                // Client message
                let message = parseData(data);
                message = JSON.parse(message);
                // console.log(message);


                // Finds client index and creates a response
                if(message.isOffer){
                    let sessionID = clientHandler.offerHandler(message, connection);
                    let buf = createMessage(sessionID);
                    connection.write(buf);
                }
                else if(message.isAnswer){
                    let session = clientHandler.answerHandler(message);
                    let buf = createMessage(JSON.stringify(session.remoteAnswer));
                    session.offerSocket.write(buf);
                }
                else if(!message.isAnswer && !message.isOffer){
                    let offerDescription = clientHandler.getOfferHandler(message);
                    let buf = createMessage(offerDescription);
                    connection.write(buf);
                };
                
            }
        });
        // Removes clients from array when they close connection
        connection.on('end', () => {
            // for (let i = 0; i < clients.length; i++) {
            //     if (clients[i] === connection) {
            //         clients.splice(i);
            //     }
            // }
            console.log("Client disconnected");
        });
    });


    wsServer.on("error", (error) => {
        console.log("Error: ", error);
    });

    wsServer.on("close", () => {

    })

    wsServer.listen(WSPORT, () => {
        console.log('Websocket server listening on port: ', WSPORT);
    });
}

/**
 * 
 * @param clientKey 
 * @returns acceptKey
 */

function createAcceptKey(clientKey) {
    acceptKey = clientKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    acceptKey = crypto.createHash("sha1").update(acceptKey, "binary").digest("base64");
    return acceptKey;
}

/**
 * 
 * @param {*} text that is being converted
 * @returns a buffer with bytes 
 */

function createMessage(text) {
    // Byte length
    let textByteLength = Buffer.from(text).length;

    // datalength bits into a single byte
    let secondByte, buffer1;
    if (textByteLength < 126) {
        secondByte = textByteLength;
        buffer1 = Buffer.from([0b10000001, secondByte]);
    }
    else if (textByteLength > 125 && textByteLength < 65535) {
        secondByte = 126;
        buffer1 = Buffer.alloc(4);
        buffer1.writeUInt8(0b10000001, 0)
        buffer1.writeUInt8(secondByte, 1);
        buffer1.writeUInt16BE(textByteLength, 2);
    }
    else {
        throw Error("Message was too long");
    }

    const buffer2 = Buffer.from(text);

    // Concatenates all buffers into one
    const buffer = Buffer.concat([buffer1, buffer2]);
    return buffer;

}

/**
 * Parses masked data from client
 * @param data masked
 * @returns parsed data in a string
 */

function parseData(data) {
    // Checks if the first bit in the second byte is a 1 or 0. Masked or not.
    let masked = data[1] >> 7 === 1;
    let length, maskStart;

    // If byte length is less than 126 the data length is equal to the 7 least significant
    // bits of the second byte.
    length = data[1] & 0b1111111;

    // Two first bits are used for meta data.
    maskStart = 2;

    if (length === 126) {
        // Creates a 16 bit number from two bytes
        length = ((data[2] << 8) | data[3]);

        // Two more bits are being used
        maskStart = 4;
    }
    else if (length === 127) {
        length = data[2];
        // Creates a 64 bit number from 8 bytes.
        for (let i = 3; i < 10; i++) {
            length = (length << 8) | data[i];
        }

        maskStart = 10;
    }
    let result = "";
    if (masked) {
        // Four bytes used for masking
        let dataStart = maskStart + 4;
        for (let i = dataStart; i < dataStart + length; i++) {
            // XORs the payload with the mask bytes going circularly.
            let byte = data[i] ^ data[maskStart + ((i - dataStart) % 4)]
            result += String.fromCharCode(byte);
        }
    } else {
        // No mask bytes, and no masking.
        for (let i = maskStart; i < maskStart + length; i++) {
            result += String.fromCharCode(data[i]);
        }
    }

    return result;

}

/**
 * Checks that alle necessary headerfields was sent by client
 * @param headers from HTTP-GET request
 * @returns boolean
 */

function checkHeaderFields(headers) {
    let connectionReg = /Connection:.+Upgrade.*?\s/i
    let hostReg = /Host:/i
    let upgradeReg = /Upgrade:.+websocket.*?\s/i
    let keyReg = /Sec-WebSocket-Key:/i
    let versionReg = /Sec-WebSocket-Version: 13\s/i
    if (connectionReg.test(headers) && hostReg.test(headers) && upgradeReg.test(headers) && keyReg.test(headers) && versionReg.test(headers))
        return true;

    return false;
}
