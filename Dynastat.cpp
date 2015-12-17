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
                    Json::Value conf = motorConfig[itr.name()];
                    if (conf == false or conf.get("address", 0).asInt()) {
                        continue;
                    }
                    RMCS220xMotor* motor = new RMCS220xMotor(
                            conf["low"].asInt(),
                            conf["high"].asInt(),
                            conf["address"].asInt(),
                            conf.get("bus", -1).asInt(),
                            conf["speed"].asInt(),
                            conf["damping"].asInt()
                    );
                    motors[itr.key().asString()] = motor;
                }

                // @TODO: Rince and repeat for sensors
            }

                break;

            default:
                break;
        }
    }

    RMCS220xMotor::RMCS220xMotor(int rawLow, int rawHigh, int address, int bus, int speed, int damping) {
        this->rawLow = rawLow;
        this->rawHigh = rawHigh;
        return;
    }

    int RMCS220xMotor::getPosition() {
        return 27;
    }

    void RMCS220xMotor::setPosition(int pos) {
        return;
    }

    int RMCS220xMotor::scalePos(int val, bool up) {
        return 0;
    }

    int RMCS220xMotor::translateValue(int val, int leftMin, int leftMax, int rightMin, int rightMax) {
        return 0;
    }
}
