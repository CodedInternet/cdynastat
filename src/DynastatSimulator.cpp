//
// Created by Tom Price on 17/12/2015.
//
#include "DynastatSimulator.h"

#include <boost/random.hpp>

namespace dynastat {

    Dynastat::Dynastat(YAML::Node config) {
        switch (config["version"].as<int>()) {
            case 1: {
                // setup sensors
                YAML::Node sensorConfig = config["sensors"];
                for (YAML::const_iterator it = sensorConfig.begin(); it != sensorConfig.end(); ++it) {
                    YAML::Node conf = it->second;
                    if (!conf[kConfAddress].IsDefined() && conf[kConfAddress].as<int>() <= 0) {
                        std::cerr << "Cannot setup sensor " << it->first.as<std::string>() << std::endl;
                        continue;
                    }

                    SimulatedSensor *sensor = new SimulatedSensor(
                            conf["registry"].as<uint>(),
                            conf["mirror"].as<bool>(false),
                            conf["rows"].as < unsigned short > (),
                            conf["cols"].as < unsigned short > (),
                            conf["zero_value"].as < unsigned short > (),
                            conf["half_value"].as < unsigned short > (),
                            conf["full_value"].as < unsigned short > ()
                    );

                    sensors[it->first.as<std::string>()] = sensor;
                }
                // setup motors

                YAML::Node motorConfig = config["motors"];
                for (YAML::const_iterator it = motorConfig.begin(); it != motorConfig.end(); ++it) {
                    YAML::Node conf = it->second;
                    if (!conf[kConfAddress].IsDefined() && conf[kConfAddress].as<int>() <= 0) {
                        std::cerr << "Cannot setup motor " << it->first.as<std::string>() << std::endl;
                        continue;
                    }

                    SimulatedMotor *motor = new SimulatedMotor(
                            conf[kConfAddress].as<int>(),
                            conf[kConfCal].as<int>(),
                            conf[kConfLow].as<int>(),
                            conf[kConfHigh].as<int>(),
                            conf[kConfSpeed].as<int>(),
                            conf[kConfDamping].as<int>()
                    );
                    motors[it->first.as<std::string>()] = motor;
                }
                break;

            }
        }
    }

    int SimulatedMotor::getPosition() {
        return position;
    }

    void SimulatedMotor::setPosition(int pos) {
        position = pos;
        targetPosition = scalePos(position);
    }

    SimulatedSensor::SimulatedSensor(uint registry, bool mirror, unsigned short rows, unsigned short cols,
                                     unsigned short zeroValue, unsigned short halfValue, unsigned short fullValue) {
        this->rows = rows;
        this->cols = cols;
        this->zeroValue = zeroValue;

        buffer = new uint16_t[rows * cols];

        setScale(zeroValue, halfValue, fullValue);

        worker = new boost::thread(boost::bind(&SimulatedSensor::updateValue, this));
    }

    void SimulatedSensor::updateValue() {
        while (running) {

            lock.lock();

            for (unsigned short row = 0; row < rows; row++) {
                for (unsigned short col = 0; col < cols; col++) {
                    int offset = (col * rows) + row;
                    uint16_t value = buffer[offset];

                    if (rand() % 10000 == 0) {
                        value = rand() % (uint16_t) -1;
                    } else {
                        int change = rand() % (range * 2 + 1) - range;
                        if ((value + change) < 0) {
                            value = 0;
                        } else if ((value + change) > UINT16_MAX) {
                            value = UINT16_MAX;
                        } else {
                            value += change;
                        }
                    }

                    buffer[offset] = value;
                }
            }

            lock.unlock();

            boost::this_thread::sleep(boost::posix_time::milliseconds(2));
        }
    }

    SimulatedSensor::~SimulatedSensor() {
        running = false;
        worker->join();
    }

    unsigned int SimulatedSensor::getValue(int row, int col) {
        return scaleValue(buffer[getOffset(row, col)]);
    }

    SimulatedMotor::SimulatedMotor(unsigned short address, int32_t cal, int32_t low, int32_t high, int16_t speed,
                                   int16_t damping) {
        this->rawLow = low;
        this->rawHigh = high;
        this->speed = speed;

        worker = new boost::thread(boost::bind(&SimulatedMotor::performMovement, this));
    }

    void SimulatedMotor::performMovement() {
        const int sf = 10;
        while (running) {
            int diff = targetPosition - currentPosition;
            if (abs(diff) > speed / sf) {
                if (diff < 0) {
                    diff = -speed / sf;
                } else {
                    diff = speed / sf;
                }
            }
            currentPosition += diff;

            boost::this_thread::sleep(boost::posix_time::milliseconds(20));
        }
    }

    SimulatedMotor::~SimulatedMotor() {
        running = false;
        worker->join();
    }

    int SimulatedMotor::getCurrentPosition() {
        return scalePos(currentPosition, false);
    }
}
