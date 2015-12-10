#define WEBRTC_POSIX

#include "flagdefs.h"

#include <iostream>
#include <string>
#include "webrtc/base/thread.h"
#include "webrtc/base/scoped_ptr.h"
#include "webrtc/base/json.h"
#include "webrtc/base/ssladapter.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/peerconnection.h"

// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";

class DummySetSessionDescriptionObserver
        : public webrtc::SetSessionDescriptionObserver {
public:
    static DummySetSessionDescriptionObserver* Create() {
        return
                new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
    }
    virtual void OnSuccess() {
        LOG(INFO) << __FUNCTION__;
    }
    virtual void OnFailure(const std::string& error) {
        LOG(INFO) << __FUNCTION__ << " " << error;
    }

protected:
    DummySetSessionDescriptionObserver() {}
    ~DummySetSessionDescriptionObserver() {}
};

class Conductor
  : public webrtc::PeerConnectionObserver,
    public webrtc::CreateSessionDescriptionObserver,
    public webrtc::DataChannelObserver {
public:
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peerConnectionFactory;
    rtc::scoped_refptr<webrtc::DataChannelInterface> outboundDatachannel;

    Conductor();

    virtual void OnStateChange();

    virtual void OnMessage(const webrtc::DataBuffer &buffer);

    virtual void OnRenegotiationNeeded();

    virtual void OnSuccess(webrtc::SessionDescriptionInterface *desc);

    virtual void OnFailure(const std::string &error);

    virtual int AddRef() const;

    virtual int Release() const;

    virtual void OnAddStream(webrtc::MediaStreamInterface *stream);

    virtual void OnRemoveStream(webrtc::MediaStreamInterface *stream);

    virtual void OnDataChannel(webrtc::DataChannelInterface *data_channel);

    virtual void OnIceCandidate(const webrtc::IceCandidateInterface *candidate);

    void OnOffer(std::string offer);
};

int main(int argc, char* argv[]) {
    rtc::FlagList::SetFlagsFromCommandLine(&argc, argv, true);
    if (FLAG_help) {
        rtc::FlagList::Print(NULL, false);
        return 0;
    }


    rtc::AutoThread autoThread;
    rtc::Thread* thread = rtc::Thread::Current();

    webrtc::PeerConnectionInterface::IceServers servers;
    webrtc::PeerConnectionInterface::IceServer server;
    server.uri = "stun.l.google.com:19302";
    servers.push_back(server);

    Conductor* conductor = new Conductor();

    std::cout << "Please paste session description:";
    std::string offer;
    getline(std::cin, offer);
    conductor->OnOffer(offer);

    thread->Run();

    std::cout << "Exiting";

    return 0;
}


int Conductor::AddRef() const {
    return 0;
}

int Conductor::Release() const {
    return 0;
}

void Conductor::OnAddStream(webrtc::MediaStreamInterface *stream) {
    return;
}

void Conductor::OnRemoveStream(webrtc::MediaStreamInterface *stream) {

}

void Conductor::OnDataChannel(webrtc::DataChannelInterface *data_channel) {
    data_channel->RegisterObserver(this);
}

void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface *candidate) {
    peerConnection->AddIceCandidate(candidate);

    const webrtc::SessionDescriptionInterface *desc = peerConnection->local_description();
    std::string sdp;
    desc->ToString(&sdp);

    Json::Value answer;
    answer["type"] = "answer";
    answer["sdp"] = sdp;

    std::cout << answer;
}

Conductor::Conductor() {
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
}

void Conductor::OnOffer(std::string offer) {
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

    std::string smessage = jmessage["message"].asString();

    std::cout << "Recieved message: " << smessage;

//    webrtc::DataBuffer out(smessage);
//    outboundDatachannel->Send(out);
}
