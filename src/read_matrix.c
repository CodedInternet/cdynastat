#include <linux/i2c-dev.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main (int argc, char *argv[])
{
    uint8_t i2cbus;
    int i2caddr;
    int i, j;
    int length = 16 * 24 * sizeof(uint16_t);
    uint8_t *buffer;
    uint16_t *ptr;
    int fd;
    char filename[256];

    if (argc != 3)
    {
        printf("Usage: %s <i2c bus> <i2c address>\n", argv[0]);
        return -1;
    }

    i2cbus = strtol(argv[1], NULL, 16);
    i2caddr = strtol(argv[2], NULL, 16);

    snprintf(filename, 256, "/dev/i2c-%d", i2cbus);
    fd = open(filename, O_RDWR);
    if (fd < 0) {
        printf("Unable to open I2C bus %d\n", i2cbus);
        exit(1);
    }

    if (ioctl(fd, I2C_SLAVE, i2caddr) < 0) {
        printf("Unable to open I2C device 0x%02X\n", i2caddr);
        exit(1);
    }

    printf("Dumping contents of device 0x%02X on bus %d\n", i2caddr, i2cbus);

    buffer = malloc(length);
    ptr = buffer;

    // Set 16x10 mode.
    buffer[0] = 0x00;
    buffer[1] = 0x01;
//    buffer[2] = 0x00;		// 1:1 mapping
//    buffer[2] = 0x01;		// 16x10 and 6x12
    buffer[2] = 0x02;		// 12x12
    write(fd, buffer, 3);


    // Set the start address.
    buffer[0] = 0x00;
    buffer[1] = 0x00;
    write(fd, buffer, 2);
    read(fd, buffer, 0x10);

    for (i=0; i<0x5; ++i)
    {
        printf("0x%04X ", ptr[i]);
    }
    printf("\n");

    // Set the start address.
    buffer[0] = 0x01;
    buffer[1] = 0x00;
    write(fd, buffer, 2);
    read(fd, buffer, length);

    printf(" ----------------------------------------------------------------- ---------------------------------\n");
    printf("|");
    for (i=0, j=1; i<length/2; ++i, ++j)
    {
        if (*ptr == 0)
        {
            ptr++;
            printf("    ");
        }
        else
            printf("%03X ", *ptr++);
        if ((i+1) % 24 == 0)
        {
            j = 0;
            printf("|\n|");
        }

        if (j == 16)
        {
            printf(" | ");
        }
    }
    printf("----------------------------------------------------------------- ---------------------------------\n");
    printf("\n");

    free(buffer);

    return 0;
}



