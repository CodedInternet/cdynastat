#include "flagdefs.h"

#include <iostream>
#include <fstream>
#include <json/reader.h>

#include "conductor.h"
#include "Dynastat.h"
//#include "DynastatSimulator.h"
#include "PeerConnectionClient.h"


int main(int argc, char *argv[]) {
    rtc::FlagList::SetFlagsFromCommandLine(&argc, argv, true);
    if (FLAG_help) {
        rtc::FlagList::Print(NULL, false);
        return 0;
    }

    Json::Value root;
    Json::Reader reader;
    std::ifstream config_doc("bbb_config.json", std::ifstream::binary);
    if (!config_doc.is_open()) {
        std::cerr << "Error opening config file" << std::endl;
        return 1;
    }

    bool configOk = reader.parse(config_doc, root, false);
    if (!configOk) {
        std::cerr << "Could not parse config file";
        return 1;
    }

    std::shared_ptr<dynastat::Dynastat> device = std::make_shared<dynastat::Dynastat>(root);

    std::shared_ptr<dynastat::PeerConnectionClient> connectionClient = std::make_shared<dynastat::PeerConnectionClient>();
    dynastat::ConductorFactory* conductorFactory = new dynastat::ConductorFactory(connectionClient, device);

    const Json::Value signaingServers = root["signaling_servers"];
    for (Json::ValueIterator itr = signaingServers.begin(); itr != signaingServers.end(); itr++) {
        Json::Value server = signaingServers[itr.index()];
        int id = connectionClient->connect(server.asString());
        connectionClient->add_listener(id, conductorFactory);
    }

    rtc::Thread *thread = new rtc::Thread;
    thread->Start(conductorFactory);

    bool running = true;
    std::string input;
    while(running) {
        getline(std::cin, input);
        if (input == "quit" || input == "q" || input == "exit") {
            running = false;
            break;
        }
    }

    std::cout << "Exiting";

    return 0;
}
