#define WEBRTC_POSIX

#include "flagdefs.h"

#include <iostream>
#include <fstream>
#include <json/reader.h>

#include "conductor.h"
#include "DynastatSimulator.h"

static enum cmdValue {
  eExit,
  eOffer
};

static std::map<std::string, cmdValue> cmdMap;

static void Initalize();

int main(int argc, char *argv[]) {
  Initalize();

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

  dynastat::DynastatSimulator* device = new dynastat::DynastatSimulator(root);

  webrtc::PeerConnectionInterface::IceServers servers;
  webrtc::PeerConnectionInterface::IceServer server;
  server.uri = "stun:stun.l.google.com:19302";
  servers.push_back(server);

  std::string cmd;
  bool running = true;
  while(running) {
    std::cin >> cmd;
    switch(cmdMap[cmd]) {
      case eExit:
        running = false;
        break;

      default:
        std::cout << "Unkown command \"" << cmd << "\". Type exit to quit.";
    }
  }

  std::cout << "Exiting";

  return 0;
}

static void Initalize() {
  cmdMap["exit"] = eExit;
  cmdMap["q"] = eExit;
  cmdMap["quit"] = eExit;
}