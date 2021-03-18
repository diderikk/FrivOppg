const ws = new WebSocket('ws://localhost:3001');
let offer = null;
let remoteDescription = null;

const configuration = { iceServers: [{ urls: 'stun:stun1.l.google.com:19302' }] }
const WebRTCConnection = new RTCPeerConnection(configuration);

const chatChannel = WebRTCConnection.createDataChannel('chat');
chatChannel.onmessage = (event) => console.log('onmessage:', event.data);
chatChannel.onopen = () => console.log('onopen');
chatChannel.onclose = () => console.log('onclose');

WebRTCConnection.onicecandidate = (event) => {

    if (((/srflx/).test(WebRTCConnection.localDescription.toJSON().sdp)) && !offer) {
        console.log('localDescription:', JSON.stringify(WebRTCConnection.localDescription));
        offer = WebRTCConnection.localDescription;
        sendOffer(offer);
    }
};

window.onload = () => {

    document.getElementById('create').onclick = () => {
        createRoom();
    }

    document.getElementById('connect').onclick = () => {
        ws.send('G');
    }

    if (remoteDescription) {

    }


}

ws.onmessage = event => {
    console.log(event.data);
    if (event.data === 'Created a channel') return;
    joinRoom(JSON.parse(event.data));

}

function createRoom() {

    WebRTCConnection.createOffer().then((localDescription) => {
        WebRTCConnection.setLocalDescription(localDescription);
    });
}

function joinRoom(remoteDescription) {

    const WebRTCConnection = new RTCPeerConnection({
        iceServers: [
            {
                urls: 'stun:stun1.l.google.com:19302',
            },
        ],
    });

    let chatChannel;
    WebRTCConnection.ondatachannel = (event) => {
        if (event.channel.label == 'chat') {
            chatChannel = event.channel;
            chatChannel.onmessage = (event) => console.log('onmessage:', event.data);
            chatChannel.onopen = () => console.log('onopen');
            chatChannel.onclose = () => console.log('onclose');
        }
    };

    WebRTCConnection.onicecandidate = (event) => {
        if (event.candidate)
            console.log('localDescription:', JSON.stringify(WebRTCConnection.localDescription));
    };

    WebRTCConnection.setRemoteDescription(remoteDescription);

    WebRTCConnection.createAnswer().then((localDescription) => {
        WebRTCConnection.setLocalDescription(localDescription);
    });


}

function sendOffer(data) {
    ws.send(JSON.stringify(data));
}

function getOffer() {
    ws.send('GetRemoteOffer');
}