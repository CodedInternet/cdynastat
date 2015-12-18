//
// Created by Tom Price on 15/12/2015.
//

#ifndef CDYNASTAT_DYNASTAT_H
#define CDYNASTAT_DYNASTAT_H

#include <json/reader.h>
#include "AbstractDynastat.h"

namespace dynastat {
class Dynastat: public AbstractDynastat {

 public:
  Dynastat(Json::Value &config);
};

class RMCS220xMotor: public AbstractMotor {

 public:
  RMCS220xMotor(int rawLow, int rawHigh, int address, int bus, int speed, int damping);

  virtual int getPosition();

  virtual void setPosition(int pos);

 private:

  virtual int scalePos(int val, bool up);

  virtual int translateValue(int val, int leftMin, int leftMax, int rightMin, int rightMax);
};
}


#endif //CDYNASTAT_DYNASTAT_H
