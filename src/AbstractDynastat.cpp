#include <string>
#include <cmath>
#include <json/value.h>
#include <assert.h>

#include "AbstractDynastat.h"

namespace dynastat {

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
            if (val < 0 or val > pow(2, bits)) {
                throw std::invalid_argument(
                        "Value " + std::to_string(val) + " is not in the range 0, " + std::to_string(pow(2, bits) - 1));
            }

            return translateValue(val, 0, (int) (pow(2, bits) - 1), rawLow, rawHigh);
        } else {
            if (val < rawLow) {
                val = rawLow;
            } else if (val > rawHigh) {
                val = rawHigh;
            }

            return translateValue(val, rawLow, rawHigh, 0, (int) (pow(2, bits) - 1));
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

    void AbstractSensor::setScale(uint16_t zeroValue, uint16_t halfValue, uint16_t fullValue) {
        this->zeroValue = zeroValue;

        double max = (pow(2, bits) - 1);
        double m1 = halfValue / (max / 2);
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

    std::map<std::string, AbstractSensor::sensorState> AbstractDynastat::readSensors() {
        std::map<std::string, AbstractSensor::sensorState> state;
        for (SensorMap::iterator it1 = sensors.begin(); it1 != sensors.end(); ++it1) {
            state[it1->first] = it1->second->getState();
        }
        return state;
    }

    AbstractSensor::sensorState AbstractSensor::getState() {
        sensorState state = sensorState(rows, std::vector<int>(cols, 0));
        for (unsigned short row = 0; row < rows; ++row) {
            for (unsigned short col = 0; col < cols; ++col) {
                state[row][col] = getValue(row, col);
            }
        }
        return state;
    }

    int AbstractSensor::getOffset(unsigned short row, unsigned short col) {
        // less buffer overflows
        assert(row < rows);
        assert(col < cols);

        return row * cols + col;
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

    AbstractMotor::motorState AbstractMotor::getState() {
        motorState state;
        state["target"] = getPosition();
        state["current"] = getCurrentPosition();
        return state;
    };

    std::map<std::string, AbstractMotor::motorState> AbstractDynastat::readMotors() {
        std::map<std::string, AbstractMotor::motorState> state;
        for (MotorMap::iterator it1 = motors.begin(); it1 != motors.end(); ++it1) {
            state[it1->first] = it1->second->getState();
        }
        return state;
    }

    int AbstractMotor::getPosition() {
        return position;
    }
}
