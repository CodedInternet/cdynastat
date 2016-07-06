//
// Created by Tom Price on 15/12/2015.
//

#include "conductor.h"

#include <boost/thread.hpp>
#include "talk/app/webrtc/test/fakeconstraints.h"

namespace dynastat {

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
        std::string label = data_channel->label();

        if (label == kChannelDataName) {
            this->m_tx = data_channel;
            this->m_device->addClient(this);
        } else if (label == kChannelControlName) {
            this->m_rx = data_channel;
        }
    }

    void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface *candidate) {

    }

    Conductor::Conductor(std::shared_ptr<dynastat::AbstractDynastat> device,
                         std::shared_ptr<PeerConnectionClient> connectionClient,
                         int connectionClientId) : m_device(device),
                                                   m_connectionClient(connectionClient),
                                                   m_connectionClientId(connectionClientId) {

    }

    void Conductor::OnFailure(const std::string &error) {

    }

    void Conductor::OnSuccess(webrtc::SessionDescriptionInterface *desc) {
        m_peerConnection->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);
    }

    void Conductor::OnRenegotiationNeeded() {

    }

    void Conductor::OnStateChange() {
        switch (m_peerConnection->signaling_state()) {
            case webrtc::PeerConnectionInterface::SignalingState::kClosed:
                running = false;
        }
    }

    void Conductor::OnMessage(const webrtc::DataBuffer &buffer) {
        Json::Reader reader;
        Json::Value jmessage;

        std::string message(buffer.data.data<char>());

        if (!reader.parse(message, jmessage)) {
            std::cerr << "Could not parse data channel message";
            return;
        }

        if (jmessage[kMsgCmdKey] == kMsgSetMotor) {
            std::string name = jmessage[kMsgNameKey].asString();
            int value = jmessage[kMsgValueKey].asInt();
            try {
                m_device->setMotor(name, value);
            } catch (std::out_of_range) {
                std::cerr << "Unknown motor: " << name << std::endl;
                return;
            } catch (std::invalid_argument) {
                std::cerr << "Unacceptable argument" << std::endl;
            }
        }
    }

    void Conductor::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
        if (new_state == m_peerConnection->kIceGatheringComplete) {
            const webrtc::SessionDescriptionInterface *desc = m_peerConnection->local_description();
            std::string sdp;
            desc->ToString(&sdp);

            Json::StyledWriter writer;
            Json::Value answer;
            answer[kSessionDescriptionTypeName] = kSessionAnswerName;
            answer[kSessionDescriptionSdpName] = sdp;

            m_connectionClient->send(m_connectionClientId, writer.write(answer));
        }
    }

    void ConductorFactory::on_message(int con_id, std::string msg) {
        Json::Reader reader;
        Json::Value jmessage;
        if (!reader.parse(msg, jmessage)) {
            std::cerr << "Received unknown message. " << msg;
            return;
        }

        if (jmessage.get(kSessionDescriptionTypeName, 0) == webrtc::SessionDescriptionInterface::kOffer) {
            OnOffer(jmessage, con_id);
        }
    }

    ConductorFactory::ConductorFactory(std::shared_ptr<PeerConnectionClient> c,
                                       std::shared_ptr<AbstractDynastat> device) : m_connectionClient(c),
                                                                                   m_device(device) {

    }

    void ConductorFactory::Run(rtc::Thread *thread) {
        // This needs to be done once we are in the specific thread as it runs on Thread::Current()
        m_peerConnectionFactory = webrtc::CreatePeerConnectionFactory();
        thread->Run();
    }

    void ConductorFactory::OnOffer(Json::Value offer, int con_id) {
        webrtc::SdpParseError error;
        webrtc::SessionDescriptionInterface *sessionDescription(
                CreateSessionDescription(webrtc::SessionDescriptionInterface::kOffer, offer.get(kSessionDescriptionSdpName, 0).asString(),
                                         &error));
        if (!sessionDescription) {
            std::cerr << "Can't parse received session description message. "
            << "SdpParseError was: " << error.description;
            return;
        }

        webrtc::PeerConnectionInterface::RTCConfiguration config;
        webrtc::PeerConnectionInterface::IceServer server;
        server.uri = "stun:stun.stunprotocol.prg";
        config.servers.push_back(server);

        webrtc::FakeConstraints constraints;

#if (DTLS)
        constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp,
                                "true");
        constraints.SetAllowDtlsSctpDataChannels();
# else
        constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp,
                                "false");
#endif
        Conductor *conductor = new Conductor(m_device, m_connectionClient, con_id);
        m_conductors.push_back(conductor);
        rtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection = m_peerConnectionFactory->CreatePeerConnection(
                config,
                &constraints,
                NULL,
                NULL,
                conductor);
        conductor->set_peerConnection(peerConnection);

        if (!peerConnection.get()) {
            std::cerr << "Failed to create peerConnection";
            return;
        }

        peerConnection->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), sessionDescription);

        peerConnection->CreateAnswer(conductor, NULL);
    }

    void Conductor::updateStatus() {
//        Json::Value json;
//        Json::FastWriter writer;
//        json["sensors"] = m_device->readSensors();
//        json["motors"] = m_device->readMotors();
//        webrtc::DataBuffer buffer(writer.write(json));

        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> pk(&sbuf);
        pk.pack_map(2);
        pk.pack("sensors");
        pk.pack(m_device->readSensors());
        pk.pack("motors");
        pk.pack(m_device->readMotors());
        rtc::Buffer buffer(sbuf.data(), sbuf.size());
        webrtc::DataBuffer dataBuffer(buffer, true);
        m_tx->Send(dataBuffer);
        return;
    }

    Conductor::~Conductor() {
        m_device->removeClient(this);
    }
};
