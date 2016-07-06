//
// Created by Tom Price on 15/12/2015.
//

#ifndef CDYNASTAT_CONDUCTOR_H
#define CDYNASTAT_CONDUCTOR_H

#define DTLS true

#include "webrtc/base/scoped_ptr.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/peerconnection.h"
#include "webrtc/base/json.h"
#include "AbstractDynastat.h"
#include "PeerConnectionClient.h"

namespace dynastat {
    // Names used for a SessionDescription JSON object.
    const char kSessionDescriptionTypeName[] = "type";
    const char kSessionDescriptionSdpName[] = "sdp";
    const char kSessionAnswerName[] = "answer";

    // Channel Names
    const char kChannelDataName[] = "data";
    const char kChannelControlName[] = "control";

    // Incomming message paramters
    const char kMsgCmdKey[] = "cmd";
    const char kMsgSetMotor[] = "set_motor";
    const char kMsgNameKey[] = "name";
    const char kMsgValueKey[] = "value";

    class DummySetSessionDescriptionObserver
            : public webrtc::SetSessionDescriptionObserver {
    public:
        static DummySetSessionDescriptionObserver *Create() {
            return
                    new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
        }

        virtual void OnSuccess() {
            std::cout << "Set Description Success" << std::endl;
            LOG(INFO) << __FUNCTION__;
        }

        virtual void OnFailure(const std::string &error) {
            std::cerr << "Set description error: " << error << std::endl;
            LOG(INFO) << __FUNCTION__ << " " << error;
        }

    protected:
        DummySetSessionDescriptionObserver() { }

        ~DummySetSessionDescriptionObserver() { }
    };

    /**
     * The conductor is similar to Connection Metadata in that it stores all the information about the associated
     * PeerConnectionInterface. The exception to this is that it also handels all the WebRTC callbacks and makes
     * sure signals get passed around where appropriate.
     */
    class Conductor
            : public webrtc::PeerConnectionObserver,
              public webrtc::CreateSessionDescriptionObserver,
              public webrtc::DataChannelObserver,
              public webrtc::IceObserver,
              public dynastat::DynastatObserver {
    public:

        ~Conductor();

        void updateStatus();

        virtual void OnStateChange() override;

        virtual void OnMessage(const webrtc::DataBuffer &buffer) override;

        virtual void OnRenegotiationNeeded() override;

        virtual void OnSuccess(webrtc::SessionDescriptionInterface *desc) override;

        virtual void OnFailure(const std::string &error) override;

        void set_peerConnection(const rtc::scoped_refptr<webrtc::PeerConnectionInterface> &m_peerConnection) {
            Conductor::m_peerConnection = m_peerConnection;
        }

        Conductor(std::shared_ptr<dynastat::AbstractDynastat> device,
                  std::shared_ptr<PeerConnectionClient> connectionClient,
                  int connectionClientId);

        virtual int AddRef() const override;

        virtual int Release() const override;

        virtual void OnAddStream(webrtc::MediaStreamInterface *stream) override;

        virtual void OnRemoveStream(webrtc::MediaStreamInterface *stream) override;

        virtual void OnDataChannel(webrtc::DataChannelInterface *data_channel) override;

        virtual void OnIceCandidate(const webrtc::IceCandidateInterface *candidate) override;

        virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;

    private:

        rtc::scoped_refptr<webrtc::PeerConnectionInterface> m_peerConnection;
        rtc::scoped_refptr<webrtc::DataChannelInterface> m_tx, m_rx;
        std::shared_ptr<dynastat::AbstractDynastat> m_device;
        std::shared_ptr<PeerConnectionClient> m_connectionClient;
        int m_connectionClientId;
        bool running = true;
    };

    class ConductorFactory : public PeerConnectionListener,
                             public rtc::Runnable {
    public:
        virtual void Run(rtc::Thread *thread) override;

        ConductorFactory(std::shared_ptr<PeerConnectionClient> c, std::shared_ptr<AbstractDynastat> device);

        ~ConductorFactory() { };

    private:
        void on_message(int con_id, std::string msg);

        void OnOffer(Json::Value offer, int con_id);

        std::shared_ptr<PeerConnectionClient> m_connectionClient;
        std::shared_ptr<AbstractDynastat> m_device;
        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> m_peerConnectionFactory;
        std::vector<Conductor *> m_conductors;
    };
}

#endif //CDYNASTAT_CONDUCTOR_H
