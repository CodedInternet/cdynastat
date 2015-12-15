//
// Created by Tom Price on 15/12/2015.
//

#ifndef CDYNASTAT_DYNASTATINTERFACE_H
#define CDYNASTAT_DYNASTATINTERFACE_H


#include <iostream>

namespace dynastat
{
    class SensorInterface {
    public:
        virtual int read() = 0;
        virtual int scale(int val) = 0;

    protected:
        int zeroValue;
        int halfValue;
        int fullValue;
    };

    class MotorInterface {
    public:
        virtual int get() = 0;
        virtual int set(int pos) = 0;

    protected:
        int bits = 8;
        int rawLow;
        int rawHigh;

        int position = 2 ^ bits -1 / 2;

    private:
        virtual int scalePos(int val, bool up = true) = 0;
        virtual int translateValue(int val, int leftMin, int leftMax, int rightMin, int rightMax) = 0;
    };

    class DynastatInterface {
    public:
        virtual int readSensor(std::string name) = 0;
        virtual int readMotor(std::string name) = 0;
        virtual void setMotor(std::string name, int pos) = 0;

    protected:
        std::map<std::string, MotorInterface*> motors;
        std::map<std::string, SensorInterface*> sensors;
    };
}

#endif //CDYNASTAT_DYNASTATINTERFACE_H
