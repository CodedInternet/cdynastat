//
// Created by Tom Price on 15/12/2015.
//

#ifndef CDYNASTAT_DYNASTAT_H
#define CDYNASTAT_DYNASTAT_H

#include <json/reader.h>
#include <yaml-cpp/yaml.h>

#include "AbstractDynastat.h"

namespace dynastat {
    class I2CBus {
    public:
        I2CBus(uint8_t bus);

        ~I2CBus();

        void get(int i2caddr, uint16_t command, uint8_t *buffer, size_t length);

        void getRaw(int i2caddr, uint16_t reg, uint8_t *buffer, size_t length);

        void put(int i2caddr, uint16_t command, uint8_t *buffer, size_t length);

    private:
        int connect(int i2caddr);

        int fd;

        boost::mutex lock;
    };

    class RMCS220xMotor : public AbstractMotor {

    public:
        virtual int getCurrentPosition() override;

        RMCS220xMotor(I2CBus *bus, int address, int rawLow, int rawHigh, int cal, int speed, int damping,
                              int control, I2CBus *controlBus);

        virtual void setPosition(int pos);

    private:
        uint16_t REG_MAX_SPEED = 0;
        uint16_t REG_MANUAL = 1;
        uint16_t REG_DAMPING = 2;
        uint16_t REG_CALIBRATE = 3;
        uint16_t REG_GOTO = 4;

        I2CBus *bus;
        I2CBus *controlBus;
        int address;
        uint16_t control = 1;
        const int kControlAddress = 0x20;

        int readPosition();

        void move(int pos);

        void home(int cal);

        bool readControl();
    };

    class SensorBoard {
    public:
        SensorBoard(I2CBus *bus, int address, uint mode);

        ~SensorBoard();

        uint16_t getValue(int reg);

    private:
        int address;
        I2CBus *bus;
        bool running = true;
        boost::thread *worker;
        boost::mutex lock;

        static const uint16_t REG_MODE = 0x01;
        static const int REG_VALUES = 0x0100;
        static const int kRows = 16;
        static const int kCols = 24;

        int length = kRows * kCols * sizeof(uint16_t);
        uint8_t *buffer = new uint8_t[length];
        uint16_t *vals = (uint16_t *) buffer;

        void update();
    };

    class DynastatSensor : public AbstractSensor {
    public:
        DynastatSensor(SensorBoard *board, uint registry, bool mirror, unsigned short rows, unsigned short cols,
                       unsigned short zero_value, unsigned short half_value, unsigned short full_value);

        virtual unsigned int getValue(int row, int col);

    private:
        static const int kBank1Cols = 16;
        static const int kBank2Cols = 8;
        static const int kRows = 16;
        static const int kCols = 24;

        int oCols;
        int oRows;

        SensorBoard *board;
        bool mirror;
    };

    class Dynastat : public AbstractDynastat {

    public:
        I2CBus *sensorBus;
        I2CBus *motorBus;

        Dynastat(YAML::Node config);

    private:
        std::map<int, SensorBoard *> sensorBoards;
    };
}


#endif //CDYNASTAT_DYNASTAT_H
