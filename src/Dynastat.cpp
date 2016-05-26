//
// Created by Tom Price on 15/12/2015.
//

#include "Dynastat.h"

#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>


namespace dynastat {
    I2CBus::I2CBus(uint8_t bus) {
        char filename[256];
        snprintf(filename, 256, "/dev/i2c-%d", bus);
        fd = open(filename, O_RDWR);
        if (fd < 0) {
            std::cerr << "Failed to open I2C bus " << 0;
            throw;
        }
    }

    I2CBus::~I2CBus() {
        close(fd);
    }

    void I2CBus::get(int i2caddr, uint16_t command, uint8_t *buffer, size_t length) {
        lock.lock();
        connect(i2caddr);

        i2c_smbus_read_i2c_block_data(fd, command, length, buffer);

        lock.unlock();
    }

    void I2CBus::getRaw(int i2caddr, uint16_t reg, uint8_t *buffer, size_t length) {
        lock.lock();
        connect(i2caddr);

        buffer[0] = (uint8_t) (reg >> 8 & 0xff);
        buffer[1] = (uint8_t) (reg & 0xff);
        write(fd, buffer, 2);
        read(fd, buffer, length);

        lock.unlock();
    }


    void I2CBus::put(int i2caddr, uint16_t command, uint8_t *buffer, size_t length) {
        lock.lock();
        connect(i2caddr);

        i2c_smbus_write_i2c_block_data(fd, command, length, buffer);

        lock.unlock();
    }

    int I2CBus::connect(int i2caddr) {
        if (ioctl(fd, I2C_SLAVE, i2caddr) < 0) {
            printf("Unable to open I2C device 0x%02X\n", i2caddr);
            throw;
        }
    }

    RMCS220xMotor::RMCS220xMotor(I2CBus *bus, int address, int rawLow, int rawHigh, int cal, int speed, int damping,
                                     int control, I2CBus *controlBus) {
        this->bus = bus;
        this->address = address;
        this->rawLow = rawLow;
        this->rawHigh = rawHigh;
        this->controlBus = controlBus;

        uint8_t b[2];
        b[0] = (uint8_t) (speed & 0xff);
        b[1] = (uint8_t) (speed >> 8);
        bus->put(address, REG_MAX_SPEED, b, 2);
        b[0] = (uint8_t) (damping & 0xff);
        b[1] = (uint8_t) (damping >> 8);
        bus->put(address, REG_DAMPING, b, 2);

        this->control = this->control << (control-1);

        home(cal);
        return;
    }

    int RMCS220xMotor::getPosition() {
        int pos = readPosition();
        return scalePos(pos, false);
    }

    void RMCS220xMotor::setPosition(int pos) {
        pos = scalePos(pos, true);
        move(pos);
        return;
    }

    int RMCS220xMotor::readPosition() {
        uint8_t b[4];
        int pos;
        bus->get(address, REG_CALIBRATE, b, 4);
        pos = (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | (b[0]);
        return pos;
    }

    void RMCS220xMotor::move(int pos) {
        uint8_t b[4];
        b[0] = (uint8_t) (pos & 0xff);
        b[1] = (uint8_t) ((pos >> 8) & 0xff);
        b[2] = (uint8_t) ((pos >> 16) & 0xff);
        b[3] = (uint8_t) (pos >> 24);
        bus->put(address, REG_GOTO, b, 4);
        return;
    }

    void RMCS220xMotor::home(int cal) {
        // @TODO: Use the I2C bus to home to motors correctly.
        int inc = std::abs(rawHigh - rawLow) / 10;
        if(cal < 0) {
            inc = -inc;
        }

        while(!readControl()) {
            int pos = readPosition() + inc;
            move(pos);
        }

        uint8_t b[4];
        b[0] = (uint8_t) (cal & 0xff);
        b[1] = (uint8_t) ((cal >> 8) & 0xff);
        b[2] = (uint8_t) ((cal >> 16) & 0xff);
        b[3] = (uint8_t) (cal >> 24);
        bus->put(address, REG_CALIBRATE, b, 4);

        move(0);
        return;
    }

    bool RMCS220xMotor::readControl() {
        uint8_t b[2];
        uint16_t value;
        controlBus->get(kControlAddress, 3, b, 2);
        value = (b[1] << 8) | (b[0]);

        return (~value & control);
    };

    DynastatSensor::DynastatSensor(I2CBus *bus, int address, uint mode, uint registry, unsigned short rows,
                                   unsigned short cols, unsigned short zero_value, unsigned short half_value,
                                   unsigned short full_value) {
        this->address = address;
        this->bus = bus;
        this->rows = rows;
        this->cols = cols;

        // Check the sensor ID is correct before we do anything
        bus->get(address, 0x00, buffer, 2);
        assert((buffer[1] << 8 | buffer[0]) == 0xFE01);

        buffer[0] = mode;
        bus->put(address, REG_MODE, buffer, 1);

        // calculate mapping offsets needed
        switch(registry) {
            case 1:
                oCols = (kBank1Cols - cols) / 2;
                break;

            case 2:
                oCols = kBank1Cols + (kBank2Cols - cols) / 2;
                break;

            default:
                break;
        }
        oRows = (kRows - rows) / 2;

        setScale(zero_value, half_value, full_value);

        fprintf(stdout, "Init sensor 0x%x.%d (%d, %d) in mode %d. Scale %f. \n", address, registry, rows, cols, mode, scale);
    }

    unsigned int DynastatSensor::getValue(int row, int col) {
        int reg = kBank1 + ((oRows + row) * kCols + col + oCols);
        bus->getRaw(address, reg, buffer, 2);
        uint16_t val = buffer[1] << 8 | buffer[0];
        return scaleValue(val);
    }

    Dynastat::Dynastat(Json::Value &config) {
        switch (config.get("version", "0").asInt()) {
            case 1: {
                // Open i2c busses
                const uint8_t sBus = (const uint8_t) config["i2c_bus"]["sensor"].asUInt();
                const uint8_t mBus = (const uint8_t) config["i2c_bus"]["motor"].asUInt();

                sensorBus = new I2CBus(sBus);
                motorBus = new I2CBus(mBus);

                const Json::Value motorConfig = config[kConfMotors];
                for (Json::ValueIterator itr = motorConfig.begin(); itr != motorConfig.end(); itr++) {
                    std::string name = itr.key().asString();
                    const Json::Value conf = motorConfig[name];

                    if (conf == false or !conf.get(kConfAddress, 0).asInt()) {
                        std::cerr << "Cannot setup motor " << name << std::endl;
                        continue;
                    }

                    RMCS220xMotor *motor = new RMCS220xMotor(motorBus, conf[kConfAddress].asInt(),
                                                             conf[kConfLow].asInt(), conf[kConfHigh].asInt(), conf[kConfCal].asInt(),
                                                             conf[kConfSpeed].asInt(), conf[kConfDamping].asInt(),
                                                             conf[kConfControl].asInt(), sensorBus);
                    motors[name] = motor;
                }

                const Json::Value sensorConfig = config["sensors"];
                for (Json::ValueIterator itr = sensorConfig.begin(); itr != sensorConfig.end(); itr++) {
                    std::string name = itr.key().asString();
                    const Json::Value conf = sensorConfig[name];

                    if (conf == false or !conf.get(kConfAddress, 0).asInt()) {
                        continue;
                    }

                    DynastatSensor *sensor = new DynastatSensor(
                            sensorBus,
                            conf[kConfAddress].asInt(),
                            conf["mode"].asUInt(),
                            conf["registry"].asUInt(),
                            (unsigned short) conf["rows"].asInt(),
                            (unsigned short) conf["cols"].asInt(),
                            (unsigned short) conf["zero_value"].asInt(),
                            (unsigned short) conf["half_value"].asInt(),
                            (unsigned short) conf["full_value"].asInt()
                    );

                    sensors[itr.key().asString()] = sensor;
                }
            }

                break;

            default:
                break;
        }
    }
}
