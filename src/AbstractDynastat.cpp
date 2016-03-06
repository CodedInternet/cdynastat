#include <string>
#include <cmath>
#include <json/value.h>
#include <assert.h>

#include "AbstractDynastat.h"

namespace dynastat {

int AbstractDynastat::readMotor(std::string name) {
  AbstractMotor *motor = motors.at(name);
  return motor->getPosition();
}

void AbstractDynastat::setMotor(std::string name, int pos) {
  AbstractMotor *motor = motors.at(name);
  motor->setPosition(pos);
}

int AbstractMotor::translateValue(int val, int leftMin, int leftMax, int rightMin, int rightMax) {
  // Figure out how 'wide' each range is
  float leftSpan = leftMax - leftMin;
  float rightSpan = rightMax - rightMin;

  // Convert the left range into a 0-1 range (float)
  float valueScaled = (val - leftMin) / leftSpan;

  // Convert the 0-1 range into a value in the right range.
  return (int) (rightMin + (valueScaled * rightSpan));
}

int AbstractMotor::scalePos(int val, bool up) {
  if (up) {
    if (val < 0 or val > (2 ^ bits)) {
      throw std::invalid_argument(
          "Value " + std::to_string(val) + " is not in the range 0, " + std::to_string(2 ^ bits - 1));
    }

    return translateValue(val, 0, 2 ^ bits - 1, rawLow, rawHigh);
  } else {
    if (val < rawLow or val > rawHigh) {
      throw std::invalid_argument(
          "Value " + std::to_string(val) + " is not in the range " + std::to_string(rawLow) + ", "
              + std::to_string(rawHigh));
    }

    return translateValue(val, rawLow, rawHigh, 0, 2 ^ bits - 1);
  }
}

AbstractDynastat::~AbstractDynastat() {
    // stop running subthreads
    running = false;
    clientNotifierThread->join();

  for (std::map<std::string, AbstractMotor *>::iterator it = motors.begin(); it != motors.end(); ++it) {
    delete it->second;
    motors.erase(it->first);
  }

  for (std::map<std::string, AbstractSensor *>::iterator it1 = sensors.begin(); it1 != sensors.end();
       ++it1) {
    delete it1->second;
    sensors.erase(it1->first);
  }
}

    void AbstractSensor::setScale(unsigned short zeroValue, unsigned short halfValue, unsigned short fullValue) {
  this->zeroValue = zeroValue;

  double max = (pow(2, bits) - 1);
  double m1 = halfValue / (max / 2) ;
  double m2 = fullValue / max;

  scale = (m1 + m2) / 2;
}

unsigned short AbstractSensor::scaleValue(int val) {
  int scaled;
  val -= zeroValue;

  if (val < 0) {
    scaled = 0;
  } else {
    scaled = (int) std::round(val / scale);

    if (scaled > pow(2, bits) - 1) {
      scaled = (int) (pow(2, bits) - 1);
    }
  }

  return scaled;
}
Json::Value AbstractDynastat::readSensors() {
  Json::Value result = Json::objectValue;
  for (SensorMap::iterator it1 = sensors.begin(); it1 != sensors.end(); ++it1) {
    result[it1->first] = it1->second->readAll();
  }
  return result;
}

    Json::Value AbstractSensor::readAll() {
        Json::Value result;
        for (unsigned short row = 0; row < rows; ++row) {
            result[row] = Json::Value(Json::arrayValue);
            for (unsigned short col = 0; col < cols; ++col) {
                result[row][col] = Json::Value();
                result[row][col] = getValue(row, col);
            }
        }
        return result;
    }

    int AbstractSensor::getOffset(unsigned short row, unsigned short col) {
        // less buffer overflows
        assert(row < rows);
        assert(col < cols);

        return col * rows + row;
    }

    void AbstractDynastat::notifyClients() {
        for (auto client : clients) {
            client->updateStatus();
        }
    }

    void AbstractDynastat::addClient(DynastatObserver *client) {
        clients.push_back(client);
    }

    void AbstractDynastat::removeClient(DynastatObserver *target) {
        int i = 0;
        for (auto client : clients) {
            if (client == target) {
                delete client;
                clients.erase(clients.begin() + i);
            }
            ++i;
        }
    }

    AbstractDynastat::AbstractDynastat() {
        clientNotifierThread = new boost::thread(boost::bind(&AbstractDynastat::clientNotifier, this));
    }

    void AbstractDynastat::clientNotifier() {
        while (running) {
            notifyClients();
            boost::this_thread::sleep(boost::posix_time::milliseconds(1000 / framerate));
        }
    }

    Json::Value AbstractMotor::getState() {
        Json::Value json;
        json["target"] = getPosition();
        json["current"] = getCurrentPosition();
        return json;
    }

    Json::Value AbstractDynastat::readMotors() {
        Json::Value result = Json::objectValue;
        for (MotorMap::iterator it1 = motors.begin(); it1 != motors.end(); ++it1) {
            result[it1->first] = it1->second->getState();
        }
        return result;
    }
}
