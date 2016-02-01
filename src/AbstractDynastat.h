//
// Created by Tom Price on 15/12/2015.
//

#ifndef CDYNASTAT_DYNASTATINTERFACE_H
#define CDYNASTAT_DYNASTATINTERFACE_H


#include <iostream>
#include <string>
#include <map>
#include <json/value.h>

namespace dynastat {
    class ValueError : public std::exception {

    };

    class AbstractSensor {
    public:
        AbstractSensor() { };

        virtual ~AbstractSensor() { };

        virtual int readValue() = 0;

        virtual int scaleValue(int val);

    protected:
        virtual void setScale(int zeroValue, int halfValue, int fullValue);

        const int bits = 8;
        int zeroValue;
        double scale;
    };

    class AbstractMotor {
    public:
        AbstractMotor() { };

        virtual ~AbstractMotor() { };

        virtual int getPosition() = 0;

        virtual void setPosition(int pos) = 0;

    protected:
        const int bits = 8;
        int rawLow;
        int rawHigh;

        int position = 2 ^bits - 1 / 2;

    private:
        virtual int scalePos(int val, bool up = true);

        virtual int translateValue(int val, int leftMin, int leftMax, int rightMin, int rightMax);
    };

    class AbstractDynastat {
    public:
        AbstractDynastat() { };

        virtual ~AbstractDynastat();

        virtual int readSensor(std::string name, int id);

        virtual Json::Value readSensors();

        virtual int readMotor(std::string name);

        virtual void setMotor(std::string name, int pos);

        const char *kConfSensors = "sensors";
        const char *kConfRows = "rows";
        const char *kConfCols = "cols";
        const char *kConfZeroValue = "zero_value";
        const char *kConfHalfValue = "half_value";
        const char *kConfFullValue = "full_value";
        const char *kConfBaseAddress = "base_address";

    protected:
        std::map<std::string, AbstractMotor *> motors;
        std::map<std::string, std::map<int, AbstractSensor *> *> sensors;
    };
}

#endif //CDYNASTAT_DYNASTATINTERFACE_H
