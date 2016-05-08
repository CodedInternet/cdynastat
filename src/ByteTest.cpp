//
// Created by Tom Price on 07/05/2016.
//

#include <iostream>
#include <string>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main(int argc, char *argv[]) {
    std::string input;
    while(1) {
        getline(std::cin, input);
        int pos = stoi(input);

        uint8_t b[4];
        b[0] = (uint8_t) (pos & 0xff);
        b[1] = (uint8_t) ((pos >> 8) & 0xff);
        b[2] = (uint8_t) ((pos >> 16) & 0xff);
        b[3] = (uint8_t) (pos >> 24);

        printf("0x%x \t 0x%x \t 0x%x \t 0x%x \n", b[0], b[1], b[2], b[3]);
    }
}

#pragma clang diagnostic pop