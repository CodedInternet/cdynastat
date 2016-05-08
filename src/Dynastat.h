//
// Created by Tom Price on 15/12/2015.
//

#ifndef CDYNASTAT_DYNASTAT_H
#define CDYNASTAT_DYNASTAT_H

#include <json/reader.h>
#include <mutex>

#include "AbstractDynastat.h"

namespace dynastat {
    class I2CBus {
    public:
        I2CBus(uint8_t bus);

        ~I2CBus();

        void get(int i2caddr, uint16_t regaddr, uint16_t *buffer, size_t length);

        void put(int i2caddr, uint8_t command, uint8_t *buffer, size_t length);

    private:
        int connect(int i2caddr);

        int fd;

        std::mutex lock;
    };

    class RMCS220xMotor : public AbstractMotor {

    public:
        virtual int getCurrentPosition() override {
            return 0;
        }

        RMCS220xMotor(I2CBus *bus, int address, int rawLow, int rawHigh, int speed, int damping);

        virtual int getPosition();

        virtual void setPosition(int pos);

    private:
        uint8_t REG_MAX_SPEED = 0;
        uint8_t REG_MANUAL = 1;
        uint8_t REG_DAMPING = 2;
        uint8_t REG_CALIBRATE = 3;
        uint8_t REG_GOTO = 4;

        I2CBus *bus;
        int address;

        virtual int scalePos(int val, bool up);

        virtual int translateValue(int val, int leftMin, int leftMax, int rightMin, int rightMax);

        void home();
    };

    class DynastatSensor : public AbstractSensor {
    public:
        DynastatSensor(I2CBus *bus, uint16_t address, uint mode, uint registry, unsigned short rows, unsigned short cols,
                       unsigned short zero_value, unsigned short half_value, unsigned short full_value);

        ~DynastatSensor();

        unsigned int getValue(int row, int col);

    private:
        uint16_t *buffer;
        size_t length;
        I2CBus *bus;
    };

    class Dynastat : public AbstractDynastat {

    public:
        I2CBus *sensorBus;
        I2CBus *motorBus;

        Dynastat(Json::Value &config);
    };
}


#endif //CDYNASTAT_DYNASTAT_H
