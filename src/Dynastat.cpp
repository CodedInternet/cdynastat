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

    void I2CBus::putRaw(int i2caddr, uint16_t reg, uint8_t *buffer, size_t length) {
        lock.lock();
        connect(i2caddr);

        uint8_t *command = new uint8_t[2 + length];

        command[0] = (uint8_t) (reg >> 8 & 0xff);
        command[1] = (uint8_t) (reg & 0xff);

        for (int i = 0; i < length; ++i) {
            command[i + 2] = buffer[i];
        }

        write(fd, command, length + 2);

        lock.unlock();
    }

    int I2CBus::connect(int i2caddr) {
        if (ioctl(fd, I2C_SLAVE, i2caddr) < 0) {
            printf("Unable to open I2C device 0x%02X\n", i2caddr);
            throw;
        }
    }

    UARTMCU::UARTMCU(const char *ttyName) {
        fd = open(ttyName, O_RDWR);
        if(fd < 0) {
            fprintf(stderr, "Unable to open UART file %s", ttyName);
            exit(-2);
        }

        // get old tty settings and zero new struct
        tcgetattr(fd, &oldtio);
        //bzero(&newtio, sizeof(newtio));

        // set board rate and serial settings
        newtio.c_cflag = B115200 | CS8;

        // write new settings
        tcflush(fd, TCIFLUSH);
        tcsetattr(fd, TCSANOW, &newtio);
    }

    UARTMCU::~UARTMCU() {
        tcsetattr(fd, TCSANOW, &oldtio);
        close(fd);
    }

    void UARTMCU::put(int i2caddr, uint8_t command, int32_t value) {
        lock.lock();
        int length = sprintf(buf, "M%d %d %d", i2caddr, command, value);
        send(buf, length);
        lock.unlock();
    }

    int32_t UARTMCU::get(int i2caddr, uint8_t command) {
        int value;
        lock.lock();
        int length = sprintf(buf, "M%d %d", i2caddr, command);
        send(buf, length);
        ssize_t bytes = read(fd, buf, sizeof(value));
        if (bytes > 0) {
            buf[bytes] = '\0'; // Null terminator
        }
        sscanf(buf, "%d", &value);
        return (int32_t) value;
    }

    int UARTMCU::send(char *bufPtr, int length) {
        int i;
        for(i = 0; i < length; i++) {
            write(fd, bufPtr++, 1);
            usleep(10000);
        }
        return i;
    }

    RMCS220xMotor::RMCS220xMotor(UARTMCU *bus, int address, int rawLow, int rawHigh, int cal, int speed, int damping,
                                 int control, I2CBus *controlBus) {
        this->bus = bus;
        this->address = address;
        this->rawLow = rawLow;
        this->rawHigh = rawHigh;
        this->controlBus = controlBus;

        printf("Motor %x: S%d \t D%d\n", address, speed, damping);

        bus->put(address, REG_MAX_SPEED, speed);
        bus->put(address, REG_DAMPING, damping);

        this->control = this->control << (control - 1);

        home(cal);
        return;
    }

    int RMCS220xMotor::getCurrentPosition() {
        return scalePos(readPosition(), false);
    }

    void RMCS220xMotor::setPosition(int pos) {
        position = pos;
        pos = scalePos(pos, true);
        move(pos);
        return;
    }

    int RMCS220xMotor::readPosition() {
        int pos;
        return bus->get(address, REG_CALIBRATE);
    }

    void RMCS220xMotor::move(int pos) {
        return bus->put(address, REG_GOTO, pos);
    }

    void RMCS220xMotor::home(int cal) {
        // @TODO: Use the I2C bus to home to motors correctly.
        int inc = std::abs(rawHigh - rawLow) / 10;
        if (cal < 0) {
            inc = -inc;
        }

        while (!readControl()) {
            int pos = readPosition() + inc;
            move(pos);
        }

        bus->put(address, REG_CALIBRATE, cal);

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

    SensorBoard::SensorBoard(I2CBus *bus, int address, uint mode) throw(UnrecognisedSensorException) {
        this->address = address;
        this->bus = bus;

        // Check the sensor ID is correct before we do anything
        bus->get(address, 0x00, buffer, 2);
        if ((buffer[1] << 8 | buffer[0]) != 0xFE01) {
            char *buf = new char[10];
            sprintf(buf, "0x%x", (uint16_t) (buffer[1] << 8 | buffer[0]));
            throw UnrecognisedSensorException(address, buf);
        }

        // Workaround for 16bit address space
        buffer[0] = (uint8_t) mode;
        bus->putRaw(address, REG_MODE, buffer, 1);

        worker = new boost::thread(boost::bind(&SensorBoard::update, this));
    }

    SensorBoard::~SensorBoard() {
        running = false;
        worker->join();
    }

    uint16_t SensorBoard::getValue(int reg) {
        uint16_t val = vals[reg];
        return val;
    }

    void SensorBoard::update() {
        while (running) {
            bus->getRaw(address, REG_VALUES, buffer, length);

            boost::this_thread::sleep(boost::posix_time::milliseconds(1000 / AbstractDynastat::framerate));
        }
    }

    uint8_t SensorBoard::changeAddress(uint8_t newAddr) {
        // stop background work so we have absolute control
        running = false;
        worker->join();

        if (newAddr < 0x00 || newAddr > 0x7F) {
            fprintf(stderr, "Invalid address 0x%x\n", newAddr);
            return 0;
        }

        // Get the old address and santiy check
        bus->getRaw(address, REG_ADDR, buffer, 1);
        uint8_t old = buffer[0];
        if (old != address) {
            fprintf(stderr, "Stored address 0x%x does not match current device 0x%x\n", old, address);
            return 0;
        }

        // perform write
        buffer[0] = newAddr;
        bus->putRaw(address, REG_ADDR, buffer, 1);

        // Get new address and check it has been successful
        bus->getRaw(address, REG_ADDR, buffer, 1);
        uint8_t updated = buffer[0];
        if (old == updated) {
            std::cerr << "Update was not successful" << std::endl;
            return old;
        } else if (updated != newAddr) {
            fprintf(stderr, "New value 0x%x does not match expected value 0x%x\n", updated, newAddr);
            return updated;
        }

        return newAddr;
    }

    DynastatSensor::DynastatSensor(SensorBoard *board, uint registry, bool mirror, unsigned short rows,
                                   unsigned short cols, unsigned short zero_value, unsigned short half_value,
                                   unsigned short full_value) {
        this->board = board;
        this->mirror = mirror;
        this->rows = rows;
        this->cols = cols;

        // calculate mapping offsets needed
        switch (registry) {
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
    }

    unsigned int DynastatSensor::getValue(int row, int col) {
        if (mirror) {
            row = (rows - 1) - row;
            col = (cols - 1) - col;
        }

        row += oRows;
        col += oCols;

        int reg = row * kCols + col;
        uint16_t val = board->getValue(reg);
        return scaleValue(val);
    }

    Dynastat::Dynastat(YAML::Node config) {
        switch (config["version"].as<int>()) {
            case 1: {
                // Open i2c busses
                const uint8_t sBus = (const uint8_t) config["i2c_bus"]["sensor"].as<int>();
                std::string mBus = config["uart"]["motor"].as<std::string>();
                sensorBus = new I2CBus(sBus);
                motorBus = new UARTMCU(mBus.c_str());

                YAML::Node motorConfig = config["motors"];
                for (YAML::const_iterator it = motorConfig.begin(); it != motorConfig.end(); ++it) {
                    YAML::Node conf = it->second;
                    if (!conf[kConfAddress].IsDefined() && conf[kConfAddress].as<int>() <= 0) {
                        std::cerr << "Cannot setup motor " << it->first.as<std::string>() << std::endl;
                        continue;
                    }

                    RMCS220xMotor *motor = new RMCS220xMotor(
                            motorBus,
                            conf[kConfAddress].as<int>(),
                            conf[kConfLow].as<int>(),
                            conf[kConfHigh].as<int>(),
                            conf[kConfCal].as<int>(),
                            conf[kConfSpeed].as<int>(),
                            conf[kConfDamping].as<int>(),
                            conf[kConfControl].as<int>(),
                            sensorBus
                    );

                    motors[it->first.as<std::string>()] = motor;
                }

                YAML::Node sensorConfig = config["sensors"];
                for (YAML::const_iterator it = sensorConfig.begin(); it != sensorConfig.end(); ++it) {
                    YAML::Node conf = it->second;
                    if (!conf[kConfAddress].IsDefined() && conf[kConfAddress].as<int>() <= 0) {
                        std::cerr << "Cannot setup sensor " << it->first.as<std::string>() << std::endl;
                        continue;
                    }

                    SensorBoard *board;
                    try {
                        board = sensorBoards.at(conf[kConfAddress].as<int>());
                    } catch (std::out_of_range) {
                        try {
                            board = new SensorBoard(sensorBus, conf[kConfAddress].as<int>(), conf["mode"].as<uint>());
                        } catch (UnrecognisedSensorException sensorException) {
                            std::cerr << sensorException.what() << std::endl;
                            continue;
                        }
                        sensorBoards[conf[kConfAddress].as<int>()] = board;
                    }

                    DynastatSensor *sensor = new DynastatSensor(
                            board,
                            conf["registry"].as<uint>(),
                            conf["mirror"].as<bool>(false),
                            conf["rows"].as < unsigned
                    short > (),
                            conf["cols"].as < unsigned
                    short > (),
                            conf["zero_value"].as < unsigned
                    short > (),
                            conf["half_value"].as < unsigned
                    short > (),
                            conf["full_value"].as < unsigned
                    short > ()
                    );

                    sensors[it->first.as<std::string>()] = sensor;
                }
            }
                break;

            default:
                break;
        }
    }

    const char *UnrecognisedSensorException::what() const throw() {
        char *buf = new char[50];
        sprintf(buf, "Unrecognised sensor at 0x%x. Response was %s", addr, resp);
        return buf;
    }
}
