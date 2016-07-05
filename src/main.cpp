#include "flagdefs.h"

#include <iostream>
#include <fstream>

#include "conductor.h"

#ifndef SIMULATOR
#include "Dynastat.h"
#else
#include "DynastatSimulator.h"
#endif

#include "PeerConnectionClient.h"

#include <yaml-cpp/yaml.h>

int main(int argc, char *argv[]) {
    rtc::FlagList::SetFlagsFromCommandLine(&argc, argv, true);
    if (FLAG_help) {
        rtc::FlagList::Print(NULL, false);
        return 0;
    }

    YAML::Node config = YAML::LoadFile(CONFIG_FILE);

    std::shared_ptr<dynastat::Dynastat> device = std::make_shared<dynastat::Dynastat>(config);

    std::shared_ptr<dynastat::PeerConnectionClient> connectionClient = std::make_shared<dynastat::PeerConnectionClient>();
    dynastat::ConductorFactory* conductorFactory = new dynastat::ConductorFactory(connectionClient, device);

    const YAML::Node signalingServers = config["signaling_servers"];
    assert(signalingServers.IsSequence());
    for(int i = 0; i < signalingServers.size(); ++i) {
        int id = connectionClient->connect(signalingServers[i].as<std::string>());
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
