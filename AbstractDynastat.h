//
// Created by Tom Price on 15/12/2015.
//

#ifndef CDYNASTAT_DYNASTATINTERFACE_H
#define CDYNASTAT_DYNASTATINTERFACE_H


#include <iostream>
#include <string>
#include <map>

namespace dynastat
{
    class ValueError : public std::exception {

    };

    class AbstractSensor {
    public:
        virtual ~AbstractSensor() { };
        virtual int readValue() = 0;
        virtual int scaleValue(int val);

    protected:
        const int bits = 8;
        int zeroValue;
        float scale;
    };

    class AbstractMotor {
    public:
        virtual ~AbstractMotor() { };
        virtual int getPosition() = 0;
        virtual void setPosition(int pos) = 0;

    protected:
        const int bits = 8;
        int rawLow;
        int rawHigh;

        int position = 2 ^ bits -1 / 2;

    private:
        virtual int scalePos(int val, bool up = true);
        virtual int translateValue(int val, int leftMin, int leftMax, int rightMin, int rightMax);
    };

    class AbstractDynastat {
    public:
        virtual ~AbstractDynastat();
        virtual int readSensor(std::string name);
        virtual int readMotor(std::string name);
        virtual void setMotor(std::string name, int pos);

    protected:
        std::map<std::string, AbstractMotor*> motors;
        std::map<std::string, AbstractSensor*> sensors;
    };
}

#endif //CDYNASTAT_DYNASTATINTERFACE_H
