
module.exports = {offerHandler, answerHandler, getOfferHandler};

const sessions = [];

function offerHandler(message, connection){
    offerDescription = message.remoteDescription;
    let id = Date.now();
    const remoteDescription = {
        offerSocket: connection,
        sessionID: id.toString(),
        remoteOffer: offerDescription,
        remoteAnswer: null
    };
    sessions.push(remoteDescription);
    return remoteDescription.sessionID;
}

function answerHandler(message){
    let session = findSession(message.sessionID);
    session.remoteAnswer = message.remoteDescription;
    return session;
}

function getOfferHandler(message){
    let session = findSession(message.sessionID);
    if(!session || session.remoteAnswer){
        return 'This is not a valid Session ID';
    }
    return JSON.stringify(session.remoteOffer);
}



function findSession(sessionID){
    for(session of sessions){
        if(session.sessionID === sessionID){
            return session;
        }
    }
    return null;
}