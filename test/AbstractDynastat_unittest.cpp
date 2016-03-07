//
// Created by Tom Price on 04/01/2016.
//

#include <gtest/gtest.h>
#include "AbstractDynastat.h"

namespace dynastat {

    class TestAbstractSensor : AbstractSensor {
    public:
        const float kScaleTolerance = 0.1;

        TestAbstractSensor() {
            rows = 10;
            cols = 16;
        }

        virtual int readValue() {
            return -1;
        };

        int getScaleValue(int val) {
            return scaleValue(val);
        }

        double getScale() {
            return scale;
        }

        void doSetScale(int zeroValue, int halfValue, int fullValue) {
            return setScale(zeroValue, halfValue, fullValue);
        }

        int doGetOffset(unsigned short row, unsigned short col) {
            return getOffset(row, col);
        }

    private:
        virtual unsigned int getValue(int row, int col) override {
            return 0;
        };
    };

    class TestAbstractMotor : AbstractMotor {
    public:
        TestAbstractMotor() {
            rawLow = -2550;
            rawHigh = 2550;
        }

        int doScalePos(int val, bool up = true) {
            return scalePos(val, up);
        };

        int getCurrentPosition() {
            return getPosition();
        }

        void setPosition(int pos) {
            position = pos;
        }
    };

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

        // Quick doubling check
        sensor.doSetScale(0, 255, 512);
        EXPECT_EQ(0, sensor.getScaleValue(0));
        EXPECT_EQ(127, sensor.getScaleValue(255));
        EXPECT_EQ(255, sensor.getScaleValue(512));

        // Some large realistic values - scale = ~268.55
        sensor.doSetScale(24, 36213, 64536);
        EXPECT_EQ(0, sensor.getScaleValue(24));
        EXPECT_EQ(127, sensor.getScaleValue(34105));
        EXPECT_EQ(255, sensor.getScaleValue(68480));
    }

    TEST(SensorTest, OffsetTest) {
        TestAbstractSensor sensor;

        EXPECT_EQ(0, sensor.doGetOffset(0, 0));
        EXPECT_EQ(3, sensor.doGetOffset(3, 0));
        EXPECT_EQ(10, sensor.doGetOffset(0, 1));
        EXPECT_EQ(34, sensor.doGetOffset(4, 3));
        EXPECT_EQ(159, sensor.doGetOffset(9, 15));
    }

    TEST(AbstractMotor, TestScalling) {
        TestAbstractMotor motor;

        EXPECT_EQ(-2550, motor.doScalePos(0));
        EXPECT_EQ(-10, motor.doScalePos(127));
        EXPECT_EQ(10, motor.doScalePos(128));
        EXPECT_EQ(2550, motor.doScalePos(255));

        EXPECT_EQ(255, motor.doScalePos(2550, false));
        EXPECT_EQ(0, motor.doScalePos(-2550, false));
        EXPECT_EQ(127, motor.doScalePos(0, false));
    }
}
