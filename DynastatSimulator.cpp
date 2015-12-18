//
// Created by Tom Price on 17/12/2015.
//
#include <boost/random.hpp>

#include "DynastatSimulator.h"

namespace dynastat {

    DynastatSimulator::DynastatSimulator(Json::Value config) {
        const Json::Value sensorConfig = config[kConfSensors];
        for (Json::ValueIterator itr = sensorConfig.begin() ; itr != sensorConfig.end() ; itr++ ) {
            Json::Value conf = sensorConfig[itr.name()];

            std::map<int, AbstractSensor*>* pad = new std::map<int, AbstractSensor*>;

            const int rows = conf["rows"].asInt();
            const int cols = conf["cols"].asInt();
            const int zeroValue = conf["zero_value"].asInt();
            const int halfValue = conf["half_value"].asInt();
            const int fullValue = conf["full_value"].asInt();
            const int baseAddress = conf["base_address"].asInt();

            for(int row = 0; row < rows; row++) {
                for(int col = 0; col < cols; col++) {
                    const int id = row * col;
                    SimulatedSensor* sensor = new SimulatedSensor(zeroValue, halfValue, fullValue);
                    (*pad)[id] = sensor;
                }
            }

            sensors[itr.key().asString()] = pad;

        }
    }

    int SimulatedMotor::getPosition() {
        return 0;
    }

    void SimulatedMotor::setPosition(int pos) {

    }

    SimulatedSensor::SimulatedSensor(int zeroValue, int halfValue, int fullValue) {
        this->zeroValue = zeroValue;

        int max = 2 ^ bits - 1;
        float m1 = (max/2) / halfValue;
        float m2 = max / fullValue;

        scale = (m1 + m2) / 2;
        worker = new boost::thread((boost::thread &&) [=] { updateValue(); });
    }

    int SimulatedSensor::readValue() {
        lock.lock();
        int reading = value;
        lock.unlock();
        return reading;
    }

    void SimulatedSensor::updateValue() {
        boost::random::mt19937 rng;
        boost::random::uniform_int_distribution<> range(-change,change);
        while(running) {

            lock.lock();
            value += range(rng);

            if (value < 0) {
                value = 0;
            } else if(value > (2 ^ bits - 1)) {
                value = 2 ^ bits - 1;
            }
            lock.unlock();

            boost::this_thread::sleep(boost::posix_time::seconds(1));
        }
    }

    SimulatedSensor::~SimulatedSensor() {
        running = false;
        worker->join();
    }
}
