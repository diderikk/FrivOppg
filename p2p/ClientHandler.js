module.exports = { offerHandler, answerHandler, getOfferHandler };

// Information about all session created
// Session is created by a client creating a chatChannel
const sessions = [];

/**
 * Handles client offer
 * @param {Object} messageFromClient
 * @param {Socket} connection
 * @returns Session ID
 */
function offerHandler(messageFromClient, connection) {
  offerDescription = messageFromClient.remoteDescription;
  // Creates an ID from the Date.now(), returns number of seconds from 1970
  let id = Date.now();
  // Creates  a object containing information for a chatChannel
  const remoteDescription = {
    // Connection to the client that created a chatChannel and session ID
    offerSocket: connection,
    sessionID: id.toString(),
    // LocalDescription from the client that sends a offer
    remoteOffer: offerDescription,
    // LocalDescription from the client that send answer
    remoteAnswer: null,
  };
  // Stores all sessions
  sessions.push(remoteDescription);
  return remoteDescription.sessionID;
}
/**
 * Handles client answer
 * @param {Object} messageFromClient
 * @returns a session
 */
function answerHandler(messageFromClient) {
  // Finds handler from message's sessionID
  let session = findSession(messageFromClient.sessionID);
  // Sets answer
  session.remoteAnswer = messageFromClient.remoteDescription;
  return session;
}
/**
 *
 * @param {Object} messageFromClient
 * @returns remoteAnswer
 */
function getOfferHandler(messageFromClient) {
  let session = findSession(messageFromClient.sessionID);
  // Checks if sessionID exists or if the sessionID already has received an answer(already connected)
  if (!session || session.remoteAnswer) {
    return "This is not a valid Session ID";
  }
  // Returns the offerDescription to the client that tries to connect
  return JSON.stringify(session.remoteOffer);
}

/**
 * Finds a session from all session
 * @param {number} sessionID
 * @returns {Object} session
 */
function findSession(sessionID) {
  for (session of sessions) {
    if (session.sessionID === sessionID) {
      return session;
    }
  }
  return null;
}
