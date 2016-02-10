//
// Created by Tom Price on 04/01/2016.
//

#include <gtest/gtest.h>
#include "AbstractDynastat.h"

namespace dynastat {

class TestAbstractSensor: AbstractSensor {
 public:
  const float kScaleTolerance = 0.1;

  virtual int readValue();

  int getScaleValue(int val) {
    return scaleValue(val);
  }

  double getScale() {
    return scale;
  }

  void doSetScale(int zeroValue, int halfValue, int fullValue) {
    return setScale(zeroValue, halfValue, fullValue);
  }

private:
    virtual unsigned int getValue(int row, int col) override {
      return 0;
    };
};

int TestAbstractSensor::readValue() {
  return -1;
}

TEST(SensorTest, CalculateScale) {
  TestAbstractSensor sensor;

  // Check a 1:1 calculation
  sensor.doSetScale(0, 127, 255);
  EXPECT_NEAR(1, sensor.getScale(), sensor.kScaleTolerance);

  // Should be a nice 2:1 ratio exactly
  sensor.doSetScale(0, 255, 511);
  EXPECT_NEAR(2, sensor.getScale(), sensor.kScaleTolerance);

  // Something with a non zero start point
  sensor.doSetScale(10, 137, 265);
  EXPECT_NEAR(1, sensor.getScale(), sensor.kScaleTolerance);

  // Some large realistic values
  sensor.doSetScale(24, 36213, 64536);
  EXPECT_NEAR(268.55, sensor.getScale(), sensor.kScaleTolerance);
}

TEST(SensorTest, ScaleValue) {
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
