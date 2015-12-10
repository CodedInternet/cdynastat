var pc_config = {"iceServers": [{"url": "stun:stun.l.google.com:19302"}]};

var pc = new webkitRTCPeerConnection(pc_config);

var dcOptions = {
    ordered: true,
    reliable: false
}

pc.ondatachannel = function(event) {
    receiveChannel = event.channel;
    receiveChannel.onmessage = function(event){
        console.log(event.data);
    };
};

pc.onicecandidate = function(event) {
    if(event.candidate == null) {
        console.log(JSON.stringify(pc.localDescription))
    }
}

var dc = pc.createDataChannel('data', dcOptions);

dc.onmessage = function(event) {
    console.log("Incomming message: " + event.data);
}

dc.onopen = function () {
    dc.send("Hello World!");
};

dc.onclose = function () {
    console.log("The Data Channel is Closed");
};

function getOffer() {
    pc.createOffer(function(desc) {
        pc.setLocalDescription(desc);
        console.log(JSON.stringify(desc));
    })
}

function getAnswer(offer) {
    pc.setRemoteDescription(new RTCSessionDescription(offer));
    pc.createAnswer(function(desc) {
        pc.setLocalDescription(desc);
        console.log(desc);
    })
}

function gotSignal(signal) {
    if (signal.sdp)
        pc.setRemoteDescription(new RTCSessionDescription(signal));
    else
        pc.addIceCandidate(new RTCIceCandidate(signal));
}