//
// Created by Tom Price on 31/01/2016.
//

#include <gtest/gtest.h>
#include "PeerConnectionClient.h"
#include <gmock/gmock.h>
namespace dynastat {
    class MockPeerConnectionListener : public PeerConnectionListener {
    public:
        MOCK_METHOD1(on_message, void(std::string
                msg));
    };

    TEST(PeerConnectionClientTest, EchoTest) {
        MockPeerConnectionListener mockPeerConnectionListener;

        PeerConnectionClient *peerConnectionClient = new PeerConnectionClient();
        peerConnectionClient->connect("wss://echo.websocket.org");

        peerConnectionClient->add_listener(0, &mockPeerConnectionListener);

        EXPECT_CALL(mockPeerConnectionListener, on_message(testing::_));

        // Allow 5 seconds for connection to open.
        // This could be done via signaling, but as normally we are only waiting for incomming messages, we there is no point
        // setting up a signaling architecture.
        int i = 0;
        ConnectionMetadata::Status status = peerConnectionClient->get_status(0);
        while (status != ConnectionMetadata::OPEN && i < 5) {
            sleep(1);
            status = peerConnectionClient->get_status(0);
            i++;
        }

        ASSERT_EQ(peerConnectionClient->get_status(0), ConnectionMetadata::OPEN);

        peerConnectionClient->send(0, "Hello World");
        sleep(1); // Should return in < 1sec
    };
};
