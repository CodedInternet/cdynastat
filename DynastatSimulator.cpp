//
// Created by Tom Price on 17/12/2015.
//

#include "DynastatSimulator.h"

namespace dynastat {

    DynastatSimulator::DynastatSimulator(Json::Value config) {

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
        while(running) {
            std::srand((unsigned int) std::time(0));

            lock.lock();
            value += std::rand() % (change *2 + 1) - change;

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
