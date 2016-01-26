//
// Created by Tom Price on 17/12/2015.
//
#include <boost/random.hpp>

#include "cdynastat/DynastatSimulator.h"

namespace dynastat {

DynastatSimulator::DynastatSimulator(Json::Value &config) {
  switch (config.get("version", 0).asInt()) {
    case 1: {
      const Json::Value sensorConfig = config[kConfSensors];
      for (Json::ValueIterator itr = sensorConfig.begin(); itr != sensorConfig.end(); itr++) {
        const Json::Value conf = sensorConfig[itr.key().asString()];
        if (conf == false or !conf.get(kConfBaseAddress, 0).asInt()) {
          continue;
        }

        std::map<int, AbstractSensor *> *pad = new std::map<int, AbstractSensor *>;
        const int rows = conf[kConfRows].asInt();
        const int cols = conf[kConfCols].asInt();
        const int zeroValue = conf[kConfZeroValue].asInt();
        const int halfValue = conf[kConfHalfValue].asInt();
        const int fullValue = conf[kConfFullValue].asInt();
        const int baseAddress = conf[kConfBaseAddress].asInt();

        for (int row = 0; row < rows; row++) {
          for (int col = 0; col < cols; col++) {
            // In V1 of the boards there are always 0x10 cols per row.
            const int id = row * 0x10 + col;
            SimulatedSensor *sensor = new SimulatedSensor(zeroValue, halfValue, fullValue);
            pad->insert(pad->end(), std::pair<int, AbstractSensor *>(id, sensor));
          }
        }

        sensors[itr.key().asString()] = pad;
        break;
      }

    }
  }
}

int SimulatedMotor::getPosition() {
  return 0;
}

void SimulatedMotor::setPosition(int pos) {

}

SimulatedSensor::SimulatedSensor(int zeroValue, int halfValue, int fullValue) {
  this->zeroValue = zeroValue;

  int max = (2^bits) - 1;
  float m1 = (max / 2) / halfValue;
  float m2 = max / fullValue;

  scale = (m1 + m2) / 2;
  boost::random::mt19937 rng;
  boost::random::uniform_int_distribution<> range(0, max);
  value = range(rng);
  worker = new boost::thread(boost::bind(&SimulatedSensor::updateValue, this));
}

int SimulatedSensor::readValue() {
  lock.lock();
  int reading = value;
  lock.unlock();
  boost::this_thread::sleep(boost::posix_time::milliseconds(2));
  return reading;
}

void SimulatedSensor::updateValue() {
  boost::random::mt19937 rng;
  boost::random::uniform_int_distribution<> range(-change, change);
  while (running) {

    lock.lock();
    value += range(rng);

    if (value < 0) {
      value = 0;
    } else if (value > (2 ^ bits - 1)) {
      value = 2 ^ bits - 1;
    }
    lock.unlock();

    boost::this_thread::sleep(boost::posix_time::milliseconds(5));
  }
}

SimulatedSensor::~SimulatedSensor() {
  running = false;
  worker->join();
}
}
