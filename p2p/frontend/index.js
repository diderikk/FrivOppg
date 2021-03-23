let localDescription = null;
let chatChannel = null;
let sessionID = "";
// Sets up a websocket connection to server
const ws = new WebSocket("ws://40.85.141.137:3001");

// Can be changed to stun@40.85.141.137, when server ONLINE
// TODO, change urls
const configuration = {
  iceServers: [{ urls: "stun:40.85.141.137:3478" }],
};
// Starts an PeerConnection
const WebRTCConnection = new RTCPeerConnection(configuration);

window.onload = () => {
  // Plays video from client host camera
  playVideoFromCamera();
  // When peers are connected, remote camera will be displayed
  playVideoFromRemoteCamera();

  document.getElementById("create").onclick = () => {
    ws.onmessage = (event) => {
      // If session ID is returned
      if (/^\d+/.test(event.data)) {
        document.getElementById("created-session-id").innerText =
          "Session ID: " + event.data;
        document.getElementById("created-session-id").style.visibility =
          "visible";
        document
          .getElementById("session-id-input")
          .setAttribute("disabled", "disabled");
        document.getElementById("connect").setAttribute("disabled", "disabled");
        return;
      }
      // else it received the answer
      const remoteDescription = JSON.parse(event.data);
      WebRTCConnection.setRemoteDescription(remoteDescription);
    };
    createRoom();
  };

  document.getElementById("connect").onclick = (event) => {
    event.preventDefault();
    ws.onmessage = (event) => {
      // if error occurs
      if (event.data === "This is not a valid Session ID") {
        document.getElementById("error").innerText = event.data;
        return;
      }
      // Else get offer and send answer
      document.getElementById("create").setAttribute("disabled", "disabled");
      joinRoom(JSON.parse(event.data));
    };
    sessionID = document.getElementById("session-id-input").value;
    send(sessionID, null, false, false);
  };
  // When chat is enabled
  document.getElementById("send").addEventListener("click", (event) => {
    event.preventDefault();
    let input = document.getElementById("message-input").value;
    document.getElementById("output").value += "\n" + "You: " + input;
    chatChannel.send(input);
    document.getElementById("message-input").value = "";
  });
};

/**
 * Plays video from camera and sends it to remote connection
 */
async function playVideoFromCamera() {
  try {
    const constraints = { video: true, audio: true };
    const localStream = await navigator.mediaDevices.getUserMedia(constraints);
    const video = document.getElementById("local-video");

    localStream.getTracks().forEach((track) => {
      WebRTCConnection.addTrack(track, localStream);
    });

    video.srcObject = localStream;
    video.muted = true;
  } catch (error) {
    console.error("Could not open video", error);
  }
}
/**
 * Plays video from peer's camera
 */
async function playVideoFromRemoteCamera() {
  let remoteStream = new MediaStream();
  const remoteVideo = document.getElementById("remote-video");
  remoteVideo.srcObject = remoteStream;

  WebRTCConnection.addEventListener("track", async (event) => {
    remoteStream.addTrack(event.track, remoteStream);
  });
}

/**
 * Creates offer for remote connection og creates channels
 */
function createRoom() {
  // Creates a chat channel
  chatChannel = WebRTCConnection.createDataChannel("chat");
  chatChannel.onmessage = (event) => {
    document.getElementById("output").value += "\n" + "Remote: " + event.data;
    console.log(event.data);
  };
  chatChannel.onopen = () => {
    console.log("Open");
    document.getElementById("send").removeAttribute("disabled");
    ws.close();
  };
  chatChannel.onclose = () => console.log("onclose");

  // Eventlistener for ice candidates
  WebRTCConnection.onicecandidate = (event) => {
    // If localdescription is not set or contains type srflx
    // srflx indicates an intermdiary address assigned by the STUN server
    // https://developer.mozilla.org/en-US/docs/Web/API/RTCIceCandidateType
    if (
      /srflx/.test(WebRTCConnection.localDescription.toJSON().sdp) &&
      !localDescription
    ) {
      localDescription = WebRTCConnection.localDescription;
      // Sendes offer to WebSocket
      send("", localDescription, false, true);
    }
  };
  // Creates offers
  WebRTCConnection.createOffer().then((localDescription) => {
    WebRTCConnection.setLocalDescription(localDescription);
  });
}
/**
 * Creates answers and joins channel
 * @param {Offer} remoteDescription
 */
function joinRoom(remoteDescription) {
  // Joins chat channel if offer offers it
  WebRTCConnection.ondatachannel = (event) => {
    if (event.channel.label == "chat") {
      chatChannel = event.channel;
      chatChannel.onmessage = (event) => {
        document.getElementById("output").value +=
          "\n" + "Other dude: " + event.data;
        console.log(event.data);
      };
      chatChannel.onopen = () => {
        console.log("Open");
        document.getElementById("send").removeAttribute("disabled");
        ws.close();
      };
      chatChannel.onclose = () => console.log("onclose");
    }
  };

  WebRTCConnection.onicecandidate = (event) => {
    // Finds an answer for the offer
    if (
      WebRTCConnection.localDescription.toJSON().type === "answer" &&
      !localDescription
    ) {
      localDescription = WebRTCConnection.localDescription;
      send(sessionID, localDescription, true, false);
    }
  };
  // Sets given remote description
  WebRTCConnection.setRemoteDescription(remoteDescription);

  WebRTCConnection.createAnswer().then((localDescription) => {
    WebRTCConnection.setLocalDescription(localDescription);
  });
}

/**
 * Sends an message object to the WebSocket server
 * @param {string} sessionID
 * @param {localdescription} data
 * @param {boolean} isAnswer
 * @param {boolean} isOffer
 */
function send(sessionID, data, isAnswer, isOffer) {
  let message = {
    sessionID: sessionID,
    isOffer: isOffer,
    isAnswer: isAnswer,
    remoteDescription: data,
  };
  ws.send(JSON.stringify(message));
}
