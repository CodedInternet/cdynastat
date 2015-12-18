//
// Created by Tom Price on 15/12/2015.
//

#ifndef CDYNASTAT_CONDUCTOR_H
#define CDYNASTAT_CONDUCTOR_H

#define WEBRTC_POSIX

#include <iostream>
#include "webrtc/base/thread.h"
#include "webrtc/base/scoped_ptr.h"
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
        std::cout << "Set Description Success" << std::endl;
        LOG(INFO) << __FUNCTION__;
    }
    virtual void OnFailure(const std::string& error) {
        std::cerr << "Set description error: " << error << std::endl;
        LOG(INFO) << __FUNCTION__ << " " << error;
    }

protected:
    DummySetSessionDescriptionObserver() {}
    ~DummySetSessionDescriptionObserver() {}
};

class Conductor
        : public webrtc::PeerConnectionObserver,
          public webrtc::CreateSessionDescriptionObserver,
          public webrtc::DataChannelObserver,
          public webrtc::IceObserver {
public:
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peerConnectionFactory;
    rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel;

    virtual void OnStateChange();

    virtual void OnMessage(const webrtc::DataBuffer &buffer);

    virtual void OnRenegotiationNeeded();

    virtual void OnSuccess(webrtc::SessionDescriptionInterface *desc);

    virtual void OnFailure(const std::string &error);

    Conductor(std::string offer);

    virtual int AddRef() const;

    virtual int Release() const;

    virtual void OnAddStream(webrtc::MediaStreamInterface *stream);

    virtual void OnRemoveStream(webrtc::MediaStreamInterface *stream);

    virtual void OnDataChannel(webrtc::DataChannelInterface *data_channel);

    virtual void OnIceCandidate(const webrtc::IceCandidateInterface *candidate);

    virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;

    void count();
};

#endif //CDYNASTAT_CONDUCTOR_H
