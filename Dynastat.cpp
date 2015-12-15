//
// Created by Tom Price on 15/12/2015.
//

#include "Dynastat.h"


namespace dynastat {

    Dynastat::Dynastat(Json::Value &config) {
        switch (config.get("version", "UTF-32").asInt()) {
            case 1:
            {
                const Json::Value motorConfig = config["motors"];
                for (Json::ValueIterator itr = motorConfig.begin() ; itr != motorConfig.end() ; itr++ ) {
                    Json::Value conf = motorConfig[itr.memberName()];
                    if (conf == false or conf.get("address", 0).asInt()) {
                        continue;
                    }
                    RMCS220xMotor motor(
                            conf["low"].asInt(),
                            conf["high"].asInt(),
                            conf["address"].asInt(),
                            conf.get("bus", -1).asInt(),
                            conf["speed"].asInt(),
                            conf["damping"].asInt()
                    );
                    motors[conf.asString()] = &motor;
                }
            }

                break;

            default:
                break;
        }
    }

    int Dynastat::readSensor(std::string name) {
        return 0;
    }

    int Dynastat::readMotor(std::string name) {
        return 0;
    }

    void Dynastat::setMotor(std::string name, int pos) {

    }

    RMCS220xMotor::RMCS220xMotor(int rawLow, int rawHigh, int address, int bus, int speed, int damping) {
        this->rawLow = rawLow;
        this->rawHigh = rawHigh;
        return;
    }

    int RMCS220xMotor::get() {
        return 0;
    }

    int RMCS220xMotor::set(int pos) {
        return 0;
    }

    int RMCS220xMotor::scalePos(int val, bool up) {
        return 0;
    }

    int RMCS220xMotor::translateValue(int val, int leftMin, int leftMax, int rightMin, int rightMax) {
        return 0;
    }
}
