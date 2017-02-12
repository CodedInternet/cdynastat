//
// Created by Tom Price on 15/12/2015.
//

#ifndef CDYNASTAT_DYNASTAT_H
#define CDYNASTAT_DYNASTAT_H

#include <termios.h>
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

        void putRaw(int i2caddr, uint16_t reg, uint8_t *buffer, size_t length);

    private:
        int connect(int i2caddr);

        int fd;

        boost::mutex lock;
    };

    class UARTMCU {
    public:
        UARTMCU(char* ttyName);

        ~UARTMCU();

        void put(int i2caddr, uint8_t command, int32_t value);

        int32_t get(int i2caddr, uint8_t command);

    private:
        struct termios oldtio, newtio;
        int fd;
        boost::mutex lock;
    };

    class RMCS220xMotor : public AbstractMotor {

    public:
        virtual int getCurrentPosition() override;

        RMCS220xMotor(UARTMCU *bus, int address, int rawLow, int rawHigh, int cal, int speed, int damping,
                      int control, I2CBus *controlBus);

        virtual void setPosition(int pos);

    private:
        uint8_t REG_MAX_SPEED = 0;
        uint8_t REG_MANUAL = 1;
        uint8_t REG_DAMPING = 2;
        uint8_t REG_CALIBRATE = 3;
        uint8_t REG_GOTO = 4;

        UARTMCU *bus;
        I2CBus *controlBus;
        int address;
        uint16_t control = 1;
        const int kControlAddress = 0x20;

        int readPosition();

        void move(int pos);

        void home(int cal);

        bool readControl();
    };

    class UnrecognisedSensorException : public std::exception {
    public:
        UnrecognisedSensorException(const int addr, const char* resp) : addr(addr), resp(resp) {};
        virtual const char* what() const throw();

    private:
        const int addr;
        const char* resp;
    };

    class SensorBoard {
    public:
        SensorBoard(I2CBus *bus, int address, uint mode) throw(UnrecognisedSensorException);

        ~SensorBoard();

        uint16_t getValue(int reg);

        uint8_t changeAddress(uint8_t newAddr);

    private:
        int address;
        I2CBus *bus;
        bool running = true;
        boost::thread *worker;

        static const uint16_t REG_MODE = 0x01;
        static const int REG_VALUES = 0x0100;
        static const uint16_t REG_ADDR = 0x0004;
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
