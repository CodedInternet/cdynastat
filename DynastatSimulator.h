//
// Created by Tom Price on 17/12/2015.
//

#ifndef CDYNASTAT_DYNASTATSIMULATOR_H
#define CDYNASTAT_DYNASTATSIMULATOR_H

#include <json/value.h>
#include <boost/thread/thread.hpp>
#include "AbstractDynastat.h"

namespace dynastat {
    class DynastatSimulator : public AbstractDynastat {
        DynastatSimulator(Json::Value config);
    };

    class SimulatedMotor : public AbstractMotor {

    public:
        virtual int getPosition();

        virtual void setPosition(int pos);
    };

    class SimulatedSensor : public AbstractSensor {
    public:
        virtual int readValue();

        void updateValue();

        SimulatedSensor(int zeroValue, int halfValue, int fullValue);

        ~SimulatedSensor();

    private:
        boost::thread* worker;
        std::mutex lock;
        int value;
        bool running = true;
        const int change = 5;
    };
}



#endif //CDYNASTAT_DYNASTATSIMULATOR_H
