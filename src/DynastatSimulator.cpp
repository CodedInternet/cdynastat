//
// Created by Tom Price on 17/12/2015.
//
#include <boost/random.hpp>

#include "DynastatSimulator.h"

namespace dynastat {

    DynastatSimulator::DynastatSimulator(Json::Value &config) {
        switch (config.get("version", 0).asInt()) {
            case 1: {
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

                    SimulatedSensor *sensor = new SimulatedSensor(baseAddress, rows, cols, zeroValue, halfValue, fullValue);

                    sensors[name] = sensor;
                }
                break;

            }
        }
    }

    int SimulatedMotor::getPosition() {
        return 0;
    }

    void SimulatedMotor::setPosition(int pos) {

    }

    SimulatedSensor::SimulatedSensor(unsigned short address, unsigned short rows, unsigned short cols, unsigned short zeroValue,
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
        typedef boost::random::mt19937 RNGType;
        RNGType rng;
        boost::uniform_int<> range(-change, change);
        boost::variate_generator< RNGType, boost::uniform_int<> > diff(rng, range);
        while (running) {

            lock.lock();

            for (unsigned short row = 0; row < rows; row++) {
                for (unsigned short col = 0; col < cols; col++) {
                    int offset = (col * rows) + row;
                    uint16_t value = buffer[offset];
                    int change = diff();
                    if((value + change) < 0) {
                        value = 0;
                    } else if ((value + change) > UINT16_MAX) {
                        value = UINT16_MAX;
                    } else {
                        value += change;
                    }
                    buffer[offset] = value;
                }
            }

            lock.unlock();

            boost::this_thread::sleep(boost::posix_time::milliseconds(5));
        }
    }

    SimulatedSensor::~SimulatedSensor() {
        running = false;
        worker->join();
    }

    unsigned int SimulatedSensor::getValue(int row, int col) {
        return scaleValue(buffer[getOffset(row, col)]);
    }
}
