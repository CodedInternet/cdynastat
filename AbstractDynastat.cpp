#include <string>
#include <cmath>

#include "AbstractDynastat.h"

namespace dynastat {

    int AbstractDynastat::readMotor(std::string name) {
        AbstractMotor* motor = motors.at(name);
        return motor->getPosition();
    }

    int AbstractDynastat::readSensor(std::string name) {
        AbstractSensor* sensor = sensors.at(name);
        return sensor->readValue();
    }

    void AbstractDynastat::setMotor(std::string name, int pos) {
        AbstractMotor* motor = motors.at(name);
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
            if(val < 0 or val > (2 ^ bits)) {
                throw std::invalid_argument("Value " + std::to_string(val) + " is not in the range 0, " + std::to_string(2^bits-1));
            }

            return translateValue(val, 0, 2 ^ bits - 1, rawLow, rawHigh);
        } else {
            if(val < rawLow or val > rawHigh) {
                throw std::invalid_argument("Value " + std::to_string(val) + " is not in the range " + std::to_string(rawLow) + ", " + std::to_string(rawHigh));
            }

            return translateValue(val, rawLow, rawHigh, 0, 2 ^ bits - 1);
        }
    }

    AbstractDynastat::~AbstractDynastat() {
        for (std::map<std::string, AbstractMotor *>::iterator it = motors.begin(); it != motors.end(); ++it) {
            delete it->second;
            motors.erase(it->first);
        }

        for (std::map<std::string, AbstractSensor *>::iterator it = sensors.begin(); it != sensors.end(); ++it) {
            delete it->second;
            sensors.erase(it->first);
        }
    }

    int AbstractSensor::scaleValue(int val) {
        int scaled;
        val -= zeroValue;

        if (val < 0) {
            scaled = 0;
        } else {
            scaled = (int) std::floor(val * scale);

            if (scale > (2 ^ bits - 1)) {
                scaled = 2 ^ bits - 1;
            }
        }

        return scaled;
    }
}
