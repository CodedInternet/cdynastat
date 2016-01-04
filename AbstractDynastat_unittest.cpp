//
// Created by Tom Price on 04/01/2016.
//

#include <gtest/gtest.h>
#include "AbstractDynastat.h"

namespace dynastat {

class TestAbstractSensor: AbstractSensor {
 public:

  virtual int readValue();

  int getScaleValue(int val) {
    return scaleValue(val);
  }

  void doSetScale(int zeroValue, int halfValue, int fullValue) {
    return setScale(zeroValue, halfValue, fullValue);
  }
};

int TestAbstractSensor::readValue() {
  return -1;
}

TEST(SensorTest, Scale) {
  TestAbstractSensor sensor;

  // Test a 1:1 scale
  sensor.doSetScale(0, 127, 255);
  EXPECT_EQ(0, sensor.getScaleValue(0));
  EXPECT_EQ(127, sensor.getScaleValue(127));
  EXPECT_EQ(255, sensor.getScaleValue(255));

  // check out of bounds
  EXPECT_EQ(0, sensor.getScaleValue(-1));
  EXPECT_EQ(255, sensor.getScaleValue(256));
}
}
