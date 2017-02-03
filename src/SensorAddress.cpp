#include <stdlib.h>
#include <iostream>

#include "Dynastat.h"

#define VERSION "v0.1"

int main(int argc, char *argv[]) {
    std::cout << "Sensor Address Utility " << VERSION << std::endl;

    if(argc != 4) {
        std::cerr << "Incorrect number of arguments." << std::endl;
    }

    uint8_t bus = (uint8_t) strtol(argv[1], NULL, 16);
    uint8_t current = (uint8_t) strtol(argv[2], NULL, 16);
    uint8_t target = (uint8_t) strtol(argv[3], NULL, 16);

    printf("Bus: %i - Change address 0x%x to 0x%x\n", bus, current, target);

    dynastat::I2CBus *i2cBus = new dynastat::I2CBus(bus);
    dynastat::SensorBoard *board = new dynastat::SensorBoard(i2cBus, current, 0);

    uint8_t result = board->changeAddress(target);
    if (result == 0) {
        return 1;
    }
    return 0;
}