//
// Created by Tom Price on 15/12/2015.
//

#ifndef CDYNASTAT_DYNASTATINTERFACE_H
#define CDYNASTAT_DYNASTATINTERFACE_H


#include <iostream>
#include <string>
#include <map>
#include <json/value.h>
#include <boost/thread/thread.hpp>

namespace dynastat {

    class AbstractSensor {
    public:
        AbstractSensor() { };

        virtual ~AbstractSensor() { };

        virtual unsigned short scaleValue(int val);

        virtual unsigned int getValue(int row, int col) = 0;

        virtual Json::Value readAll();

    protected:
        virtual void setScale(unsigned short zeroValue, unsigned short halfValue, unsigned short fullValue);

        virtual int getOffset(unsigned short row, unsigned short col);

        const uint8_t bits = 8;
        unsigned short address;
        unsigned short rows;
        unsigned short cols;
        unsigned short zeroValue;
        double scale;
    };

    class AbstractMotor {
    public:
        AbstractMotor() { };

        virtual ~AbstractMotor() { };

        virtual int getCurrentPosition() = 0;

        virtual int getPosition() = 0;

        virtual void setPosition(int pos) = 0;

        virtual Json::Value getState();

    protected:
        const int bits = 8;
        int rawLow;
        int rawHigh;

        int position = 2 ^bits - 1 / 2;

        virtual int scalePos(int val, bool up = true);

        virtual int translateValue(int val, int leftMin, int leftMax, int rightMin, int rightMax);
    };

    typedef std::map<std::string, AbstractSensor *> SensorMap;
    typedef std::map<std::string, AbstractMotor *> MotorMap;

    class DynastatObserver {
    public:
        virtual ~DynastatObserver() = default;

        virtual void updateStatus() = 0;
    };

    class AbstractDynastat {
    public:
        AbstractDynastat();

        virtual ~AbstractDynastat();

        virtual Json::Value readSensors();

        virtual Json::Value readMotors();

        virtual int readMotor(std::string name);

        virtual void setMotor(std::string name, int pos);

        virtual void notifyClients();

        virtual void addClient(DynastatObserver *client);

        virtual void removeClient(DynastatObserver *client);

        const char *kConfSensors = "sensors";
        const char *kConfMotors = "motors";
        const char *kConfRows = "rows";
        const char *kConfCols = "cols";
        const char *kConfZeroValue = "zero_value";
        const char *kConfHalfValue = "half_value";
        const char *kConfFullValue = "full_value";
        const char *kConfBaseAddress = "base_address";
        const char *kConfAddress = "address";
        const char *kConfCal = "cal";
        const char *kConfLow = "low";
        const char *kConfHigh = "high";
        const char *kConfSpeed = "speed";
        const char *kConfDamping = "damping";

        const int framerate = 12;

    protected:
        virtual void clientNotifier();

        MotorMap motors;
        SensorMap sensors;
        std::vector<DynastatObserver *> clients;
        bool running = true;
        boost::thread *clientNotifierThread;
    };
}

#endif //CDYNASTAT_DYNASTATINTERFACE_H
