#define WEBRTC_POSIX

#include "flagdefs.h"

#include <iostream>

#include "conductor.h"


int main(int argc, char* argv[]) {
    rtc::FlagList::SetFlagsFromCommandLine(&argc, argv, true);
    if (FLAG_help) {
        rtc::FlagList::Print(NULL, false);
        return 0;
    }

    std::cout << "Please paste session description:";

    std::string offer;
    getline(std::cin, offer);

    std::cout << "Recieved:" << offer << std::endl;


    rtc::AutoThread autoThread;
    rtc::Thread* thread = rtc::Thread::Current();

    webrtc::PeerConnectionInterface::IceServers servers;
    webrtc::PeerConnectionInterface::IceServer server;
    server.uri = "stun:stun.l.google.com:19302";
    servers.push_back(server);

    Conductor conductor(offer);

    thread->Run();

    std::cout << "Exiting";

    return 0;
}
