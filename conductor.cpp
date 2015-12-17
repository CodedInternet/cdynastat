//
// Created by Tom Price on 15/12/2015.
//

#include "conductor.h"

#include <string>

#include "webrtc/base/json.h"


int Conductor::AddRef() const {
    return 0;
}

int Conductor::Release() const {
    return 0;
}

void Conductor::OnAddStream(webrtc::MediaStreamInterface *stream) {

}

void Conductor::OnRemoveStream(webrtc::MediaStreamInterface *stream) {

}

void Conductor::OnDataChannel(webrtc::DataChannelInterface *data_channel) {
    data_channel->RegisterObserver(this);
    this->dataChannel = data_channel;
}

void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface *candidate) {
    peerConnection->AddIceCandidate(candidate);

    const webrtc::SessionDescriptionInterface* desc = peerConnection->local_description();
    std::string sdp;
    desc->ToString(&sdp);

    Json::StyledWriter writer;
    Json::Value answer;
    answer["type"] = "answer";
    answer["sdp"] = sdp;

    std::cout << writer.write(answer);
}

Conductor::Conductor(std::string offer) {
    peerConnectionFactory = webrtc::CreatePeerConnectionFactory();

    webrtc::PeerConnectionInterface::IceServers servers;
    webrtc::PeerConnectionInterface::IceServer server;
    server.uri = "stun:stun.l.google.com:19302";
    servers.push_back(server);

    peerConnection = peerConnectionFactory->CreatePeerConnection(servers,
                                                                 NULL,
                                                                 NULL,
                                                                 NULL,
                                                                 this);

    if(!peerConnection.get()) {
        std::cerr << "Failed to create peerConnection";
        return;
    }

    Json::Reader reader;
    Json::Value jmessage;
    if (!reader.parse(offer, jmessage)) {
        std::cerr << "Received unknown message. " << offer;
        return;
    }
    std::string type, sdp;
    rtc::GetStringFromJsonObject(jmessage, "type", &type);
    if(type != "offer") {
        std::cerr << "Not an offer";
        return;
    }

    rtc::GetStringFromJsonObject(jmessage, "sdp", &sdp);
    std::cout << "Using sdp: " << sdp;

    webrtc::SdpParseError error;
    webrtc::SessionDescriptionInterface* sessionDescription(webrtc::CreateSessionDescription(type, sdp, &error));
    if (!sessionDescription) {
        std::cerr << "Can't parse received session description message. "
        << "SdpParseError was: " << error.description;
        return;
    }
    peerConnection->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), sessionDescription);

    peerConnection->CreateAnswer(this, NULL);
}

void Conductor::OnFailure(const std::string &error) {

}

void Conductor::OnSuccess(webrtc::SessionDescriptionInterface *desc) {
    peerConnection->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);

    std::string sdp;
    desc->ToString(&sdp);

    Json::Value answer;
    answer["type"] = "answer";
    answer["sdp"] = sdp;

    std::cout << "Answer: " << answer;
}

void Conductor::OnRenegotiationNeeded() {

}

void Conductor::OnStateChange() {

}

void Conductor::OnMessage(const webrtc::DataBuffer &buffer) {
    Json::Reader reader;
    Json::Value jmessage;

    std::string message(buffer.data.data<char>());

    if(!reader.parse(message, jmessage)) {
        std::cerr << "Could not parse data channel message";
        return;
    }

    std::cout << "Recieved message: " << jmessage["message"] << std::endl;

    // Simple test to check if we can send through the DC as well.
    dataChannel->Send(buffer);
}