//
// Created by Tom Price on 15/12/2015.
//

#include "Dynastat.h"

#include <linux/i2c-dev.h>
#include <fcntl.h>
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

    void I2CBus::get(int i2caddr, uint16_t regaddr, uint16_t *buffer, size_t length) {
        lock.lock();
        buffer[0] = regaddr;

        if (ioctl(fd, I2C_SLAVE, i2caddr) < 0) {
            printf("Unable to open I2C device 0x%02X\n", i2caddr);
            throw;
        }

        write(fd, buffer, 1);
        read(fd, buffer, length);
        lock.unlock();
    }

    void I2CBus::put(int i2caddr, uint8_t command, uint8_t *buffer, size_t length) {
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

    RMCS220xMotor::RMCS220xMotor(I2CBus *bus, int address, int rawLow, int rawHigh, int speed, int damping) {
        this->bus = bus;
        this->address = address;
        this->rawLow = rawLow;
        this->rawHigh = rawHigh;

        uint8_t b[2];
        b[0] = (uint8_t) (speed & 0xff);
        b[1] = (uint8_t) (speed >> 8);
        bus->put(address, REG_MAX_SPEED, b, 2);
        b[0] = (uint8_t) (damping & 0xff);
        b[1] = (uint8_t) (damping >> 8);
        bus->put(address, REG_DAMPING, b, 2);

        home();
        return;
    }

    int RMCS220xMotor::getPosition() {
        return 27;
    }

    void RMCS220xMotor::setPosition(int pos) {
        pos = scalePos(pos, true);
        move(pos);
        return;
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

    void RMCS220xMotor::home() {
        // @TODO: Use the I2C bus to home to motors correctly.
        std::cout << "Homing " << address << std::endl;
        move(0);
        return;
    }

    DynastatSensor::DynastatSensor(I2CBus *bus, uint16_t address, uint mode, uint registry, unsigned short rows,
                                   unsigned short cols, unsigned short zero_value, unsigned short half_value,
                                   unsigned short full_value) {
        this->address = address;
        this->bus = bus;
        this->rows = rows;
        this->cols = cols;

        length = rows * cols * sizeof(uint16_t);
        buffer = (uint16_t *) malloc(length);

        // Check the sensor is correct
        bus->get(address, 0, buffer, 1);
        assert(buffer[0] == 0xFE01);

        setScale(zero_value, half_value, full_value);
    }

    DynastatSensor::~DynastatSensor() {
        free(buffer);
    }

    unsigned int DynastatSensor::getValue(int row, int col) {
        return 0;
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

                    RMCS220xMotor *motor = new RMCS220xMotor(motorBus,
                                                             conf[kConfAddress].asInt(),
                                                             conf[kConfLow].asInt(),
                                                             conf[kConfHigh].asInt(),
                                                             conf[kConfSpeed].asInt(),
                                                             conf[kConfDamping].asInt());
                    motors[name] = motor;
                }

                const Json::Value sensorConfig = config["sensors"];
                for (Json::ValueIterator itr = sensorConfig.begin(); itr != sensorConfig.end(); itr++) {
                    Json::Value conf = sensorConfig[itr.memberName()];
                    if (conf == false or conf.get("address", 0).asInt()) {
                        continue;
                    }

                    DynastatSensor *sensor = new DynastatSensor(
                            sensorBus,
                            (uint16_t) conf["address"].asUInt(),
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
