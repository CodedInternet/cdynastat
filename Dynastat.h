//
// Created by Tom Price on 15/12/2015.
//

#ifndef CDYNASTAT_DYNASTAT_H
#define CDYNASTAT_DYNASTAT_H

#include <json/reader.h>
#include "DynastatInterface.h"

namespace dynastat {
    class Dynastat : public DynastatInterface {

    public:
        Dynastat(Json::Value &config);

        virtual int readSensor(std::string name);

        virtual int readMotor(std::string name);

        virtual void setMotor(std::string name, int pos);
    };

    class RMCS220xMotor : public MotorInterface {

    public:
        RMCS220xMotor(int rawLow, int rawHigh, int address, int bus, int speed, int damping);

        virtual int get();

        virtual int set(int pos);

    private:

        virtual int scalePos(int val, bool up);

        virtual int translateValue(int val, int leftMin, int leftMax, int rightMin, int rightMax);
    };
}


#endif //CDYNASTAT_DYNASTAT_H
