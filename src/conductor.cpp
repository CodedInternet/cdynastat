//
// Created by Tom Price on 15/12/2015.
//

#include "conductor.h"

#include <boost/thread.hpp>
#include "webrtc/base/json.h"
#include "talk/app/webrtc/test/fakeconstraints.h"

namespace dynastat {
    void Conductor::count() {
        Json::Value json;
        json["sensors"];
        while (true) {
            json["sensors"] = device->readSensors();
            webrtc::DataBuffer buffer(json.toStyledString());
            dataChannel->Send(buffer);
            boost::this_thread::sleep(boost::posix_time::milliseconds(1));
        }
    }

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
        new boost::thread([=] { count(); });
    }

    void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface *candidate) {

    }

    Conductor::Conductor(std::shared_ptr<dynastat::AbstractDynastat> device,
                             std::shared_ptr<PeerConnectionClient> connectionClient) {
        this->device = device;
        peerConnectionFactory = webrtc::CreatePeerConnectionFactory();
        this->connectionClient = connectionClient;
        this->connectionClientId = connectionClientId;

        webrtc::PeerConnectionInterface::RTCConfiguration config;
        webrtc::PeerConnectionInterface::IceServer server;
        server.uri = "stun:stun.l.google.com:19302";
        config.servers.push_back(server);

        bool dtls = true;
        webrtc::FakeConstraints constraints;
        if (dtls) {
            constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp,
                                    "true");
            constraints.SetAllowDtlsSctpDataChannels();
        } else {
            constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp,
                                    "false");
        }

        peerConnection = peerConnectionFactory->CreatePeerConnection(config,
                                                                     &constraints,
                                                                     NULL,
                                                                     NULL,
                                                                     this);

        if (!peerConnection.get()) {
            std::cerr << "Failed to create peerConnection";
            return;
        }
    }

    void Conductor::GenerateAnswer(std::string &offer) {
        Json::Reader reader;
        Json::Value jmessage;
        if (!reader.parse(offer, jmessage)) {
            std::__1::cerr << "Received unknown message. " << offer;
            return;
        }
        std::__1::string type, sdp;
        rtc::GetStringFromJsonObject(jmessage, "type", &type);
        if (type != "offer") {
            std::__1::cerr << "Not an offer";
            return;
        }

        rtc::GetStringFromJsonObject(jmessage, "sdp", &sdp);
        std::__1::cout << "Using sdp: " << sdp;

        webrtc::SdpParseError error;
        webrtc::SessionDescriptionInterface *sessionDescription(CreateSessionDescription(type, sdp, &error));
        if (!sessionDescription) {
            std::__1::cerr << "Can't parse received session description message. "
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

//    std::string sdp;
//    desc->ToString(&sdp);
//
//    Json::Value answer;
//    answer["type"] = "answer";
//    answer["sdp"] = sdp;
//
//    std::cout << "Answer: " << answer;
    }

    void Conductor::OnRenegotiationNeeded() {

    }

    void Conductor::OnStateChange() {

    }

    void Conductor::OnMessage(const webrtc::DataBuffer &buffer) {
        Json::Reader reader;
        Json::Value jmessage;

        std::string message(buffer.data.data<char>());

        if (!reader.parse(message, jmessage)) {
            std::cerr << "Could not parse data channel message";
            return;
        }

        std::cout << "Recieved message: " << jmessage["message"] << std::endl;

        // Simple test to check if we can send through the DC as well.
        dataChannel->Send(buffer);
    }

    void Conductor::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
        if (new_state == peerConnection->kIceGatheringComplete) {
            const webrtc::SessionDescriptionInterface *desc = peerConnection->local_description();
            std::string sdp;
            desc->ToString(&sdp);

            Json::StyledWriter writer;
            Json::Value answer;
            answer["type"] = "answer";
            answer["sdp"] = sdp;

            connectionClient->send(connectionClientId, writer.write(answer));
        }
    }

    void ConductorFactory::on_message(int con_id, std::string msg) {
        webrtc::PeerConnectionInterface::IceServers servers;
        webrtc::PeerConnectionInterface::IceServer server;
        server.uri = "stun:stun.l.google.com:19302";
        servers.push_back(server);

        conductor->GenerateAnswer(msg);
    }

    ConductorFactory::ConductorFactory(std::shared_ptr<PeerConnectionClient> c,
                                       std::shared_ptr<AbstractDynastat> device) : m_connectionClient(c),
                                                                                   m_device(device) {
        conductor = new Conductor(device, c);
    }
};
