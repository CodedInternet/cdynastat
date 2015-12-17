#define WEBRTC_POSIX

#include "flagdefs.h"

#include <iostream>
#include <fstream>
#include <json/reader.h>

#include "conductor.h"
#include "Dynastat.h"


int main(int argc, char* argv[]) {
    rtc::FlagList::SetFlagsFromCommandLine(&argc, argv, true);
    if (FLAG_help) {
        rtc::FlagList::Print(NULL, false);
        return 0;
    }

    Json::Value root;
    Json::Reader reader;
    std::ifstream config_doc("bbb_config.json", std::ifstream::binary);
    if(!config_doc.is_open()) {
        std::cerr << "Error opening config file" << std::endl;
        return 1;
    }

    bool configOk = reader.parse(config_doc, root, false);
    if(!configOk) {
        std::cerr << "Could not parse config file";
        return 1;
    }

    dynastat::Dynastat device(root);

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
