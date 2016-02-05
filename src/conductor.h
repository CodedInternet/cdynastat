//
// Created by Tom Price on 15/12/2015.
//

#ifndef CDYNASTAT_CONDUCTOR_H
#define CDYNASTAT_CONDUCTOR_H

#include <iostream>
#include <queue>
#include "webrtc/base/thread.h"
#include "webrtc/base/scoped_ptr.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/peerconnection.h"
#include "AbstractDynastat.h"
#include "PeerConnectionClient.h"

namespace dynastat {
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

    class Conductor
            : public webrtc::PeerConnectionObserver,
              public webrtc::CreateSessionDescriptionObserver,
              public webrtc::DataChannelObserver,
              public webrtc::IceObserver {
    public:

        virtual void OnStateChange() override;

        virtual void OnMessage(const webrtc::DataBuffer &buffer) override;

        virtual void OnRenegotiationNeeded() override;

        virtual void OnSuccess(webrtc::SessionDescriptionInterface *desc) override;

        virtual void OnFailure(const std::string &error) override;

        Conductor(std::shared_ptr<dynastat::AbstractDynastat> device,
                          std::shared_ptr<PeerConnectionClient> connectionClient);

        virtual int AddRef() const override;

        virtual int Release() const override;

        virtual void OnAddStream(webrtc::MediaStreamInterface *stream) override;

        virtual void OnRemoveStream(webrtc::MediaStreamInterface *stream) override;

        virtual void OnDataChannel(webrtc::DataChannelInterface *data_channel) override;

        virtual void OnIceCandidate(const webrtc::IceCandidateInterface *candidate) override;

        virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;

        void count();

        void GenerateAnswer(std::string &offer);

    private:
        rtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection;
        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peerConnectionFactory;
        rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel;
        std::shared_ptr<dynastat::AbstractDynastat> device;
        std::shared_ptr<PeerConnectionClient> connectionClient;
        int connectionClientId;

    };

    class ConductorFactory : public PeerConnectionListener {
    public:
        ConductorFactory(std::shared_ptr<PeerConnectionClient> c, std::shared_ptr<AbstractDynastat> device);

        ~ConductorFactory() { };

    private:
        virtual void on_message(int con_id, std::string msg);

    private:
        std::shared_ptr<PeerConnectionClient> m_connectionClient;
        std::shared_ptr<AbstractDynastat> m_device;
        Conductor *conductor;
    };
}

#endif //CDYNASTAT_CONDUCTOR_H
