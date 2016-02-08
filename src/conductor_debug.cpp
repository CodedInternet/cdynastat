//
// Created by Tom Price on 08/02/2016.
//

#include <iostream>
#include <fstream>
#include <json/reader.h>
#include "conductor.h"
#include "DynastatSimulator.h"

int main(int argc, char* argv[]) {
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

    std::shared_ptr<dynastat::DynastatSimulator> device = std::make_shared<dynastat::DynastatSimulator>(root);
    dynastat::Conductor* conductor = new dynastat::Conductor(device, nullptr);

    rtc::AutoThread autoThread;
    rtc::Thread *thread = rtc::Thread::Current();

    std::string offer;
    std::cout << "Paste Offer: ";
    getline(std::cin, offer);
    conductor->GenerateAnswer(offer);
    thread->Run();

    return 1;
}