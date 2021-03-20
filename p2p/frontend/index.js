const ws = new WebSocket("ws://localhost:3001");
let localDescription = null;
let chatChannel = null;
let sessionID = "";

const configuration = {
  iceServers: [{ urls: "stun:stun1.l.google.com:19302" }],
};
const WebRTCConnection = new RTCPeerConnection(configuration);

window.onload = () => {
  playVideoFromCamera();
  playVideoFromRemoteCamera();

  document.getElementById("create").onclick = () => {
    ws.onmessage = (event) => {
      // console.log(event.data);
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
      const remoteDescription = JSON.parse(event.data);
      WebRTCConnection.setRemoteDescription(remoteDescription);
    };
    createRoom();
  };

  document.getElementById("connect").onclick = (event) => {
    event.preventDefault();
    ws.onmessage = (event) => {
      // console.log(event.data);
      if (event.data === "This is not a valid Session ID") {
        document.getElementById("error").innerText = event.data;
        return;
      }
      document.getElementById("create").setAttribute("disabled", "disabled");
      joinRoom(JSON.parse(event.data));
    };
    sessionID = document.getElementById("session-id-input").value;
    send(sessionID, null, false, false);
  };

  document.getElementById("send").addEventListener("click", (event) => {
    event.preventDefault();
    let input = document.getElementById("message-input").value;
    document.getElementById("output").value += "\n" + "You: " + input;
    chatChannel.send(input);
    document.getElementById("message-input").value = "";
  });
};

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

async function playVideoFromRemoteCamera() {
  let remoteStream = new MediaStream();
  const remoteVideo = document.getElementById("remote-video");
  remoteVideo.srcObject = remoteStream;

  WebRTCConnection.addEventListener("track", async (event) => {
    remoteStream.addTrack(event.track, remoteStream);
  });
}

function createRoom() {
  chatChannel = WebRTCConnection.createDataChannel("chat");
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

  WebRTCConnection.onicecandidate = (event) => {
    if (
      /srflx/.test(WebRTCConnection.localDescription.toJSON().sdp) &&
      !localDescription
    ) {
      // console.log('localDescription:', JSON.stringify(WebRTCConnection.localDescription));
      localDescription = WebRTCConnection.localDescription;
      send("", localDescription, false, true);
    }
  };

  WebRTCConnection.createOffer().then((localDescription) => {
    WebRTCConnection.setLocalDescription(localDescription);
  });
}

function joinRoom(remoteDescription) {
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
    if (
      WebRTCConnection.localDescription.toJSON().type === "answer" &&
      !localDescription
    ) {
      // console.log(JSON.stringify(WebRTCConnection.localDescription));
      localDescription = WebRTCConnection.localDescription;
      send(sessionID, localDescription, true, false);
    }
  };

  WebRTCConnection.setRemoteDescription(remoteDescription);

  WebRTCConnection.createAnswer().then((localDescription) => {
    WebRTCConnection.setLocalDescription(localDescription);
  });
}

function send(sessionID, data, isAnswer, isOffer) {
  let message = {
    sessionID: sessionID,
    isOffer: isOffer,
    isAnswer: isAnswer,
    remoteDescription: data,
  };
  ws.send(JSON.stringify(message));
}
