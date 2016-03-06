//
// Created by Tom Price on 17/12/2015.
//
#include <boost/random.hpp>

#include "DynastatSimulator.h"

namespace dynastat {

    DynastatSimulator::DynastatSimulator(Json::Value &config) {
        switch (config.get("version", 0).asInt()) {
            case 1: {
                // setup sensors
                const Json::Value sensorConfig = config[kConfSensors];
                for (Json::ValueIterator itr = sensorConfig.begin(); itr != sensorConfig.end(); itr++) {
                    std::string name = itr.key().asString();
                    const Json::Value conf = sensorConfig[name];
                    if (conf == false or !conf.get(kConfBaseAddress, 0).asInt()) {
                        continue;
                    }

                    const unsigned short rows = (const unsigned short) conf[kConfRows].asUInt();
                    const unsigned short cols = (const unsigned short) conf[kConfCols].asUInt();
                    const unsigned short zeroValue = (const unsigned short) conf[kConfZeroValue].asUInt();
                    const unsigned short halfValue = (const unsigned short) conf[kConfHalfValue].asUInt();
                    const unsigned short fullValue = (const unsigned short) conf[kConfFullValue].asUInt();
                    const unsigned short baseAddress = (const unsigned short) conf[kConfBaseAddress].asUInt();

                    SimulatedSensor *sensor = new SimulatedSensor(baseAddress, rows, cols, zeroValue, halfValue,
                                                                  fullValue);

                    sensors[name] = sensor;
                }
                // setup motors

                const Json::Value motorConfig = config[kConfMotors];
                for (Json::ValueIterator itr = motorConfig.begin(); itr != motorConfig.end(); itr++) {
                    std::string name = itr.key().asString();
                    const Json::Value conf = motorConfig[name];
                    if (conf == false or !conf.get(kConfAddress, 0).asInt()) {
                        continue;
                    }

                    const unsigned short address = (const unsigned short) conf[kConfAddress].asUInt();
                    const int32_t cal = conf[kConfCal].asInt();
                    const int32_t low = conf[kConfLow].asInt();
                    const int32_t high = conf[kConfHigh].asInt();
                    const int16_t speed = (const int16_t) conf[kConfSpeed].asInt();
                    const int16_t damping = (const int16_t) conf[kConfDamping].asInt();

                    SimulatedMotor *motor = new SimulatedMotor(address, cal, low, high, speed, damping);
                    motors[name] = motor;
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

    SimulatedSensor::SimulatedSensor(unsigned short address, unsigned short rows, unsigned short cols,
                                     unsigned short zeroValue,
                                     unsigned short halfValue,
                                     unsigned short fullValue) {
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
        while (running) {
            int diff = targetPosition - currentPosition;
            if (abs(diff) > speed) {
                if (diff < 0) {
                    diff = -speed;
                } else {
                    diff = speed;
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
