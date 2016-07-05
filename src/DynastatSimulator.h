//
// Created by Tom Price on 17/12/2015.
//

#ifndef CDYNASTAT_DYNASTATSIMULATOR_H
#define CDYNASTAT_DYNASTATSIMULATOR_H

#include "AbstractDynastat.h"
#include <boost/thread/thread.hpp>
#include <mutex>
#include <yaml-cpp/yaml.h>

namespace dynastat {
    class Dynastat : public AbstractDynastat {
    public:
        Dynastat(YAML::Node config);
    };

    class SimulatedMotor : public AbstractMotor {

    public:
        SimulatedMotor(unsigned short address, int32_t cal, int32_t low, int32_t high, int16_t speed, int16_t damping);

        ~SimulatedMotor();

        virtual int getCurrentPosition();

        virtual int getPosition();

        virtual void setPosition(int pos);

    private:
        void performMovement();

        int16_t speed;
        int32_t currentPosition;
        int32_t targetPosition;

        bool running = true;
        boost::thread *worker;
    };

    class SimulatedSensor : public AbstractSensor {
    public:
        virtual unsigned int getValue(int row, int col) override;

        void updateValue();

        SimulatedSensor(uint registry, bool mirror, unsigned short rows, unsigned short cols,
                                unsigned short zeroValue, unsigned short halfValue, unsigned short fullValue);

        ~SimulatedSensor();

    private:
        boost::thread *worker;
        std::mutex lock;
        bool running = true;
        const int range = 200;
        uint16_t *buffer;
    };
}


#endif //CDYNASTAT_DYNASTATSIMULATOR_H
