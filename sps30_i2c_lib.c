#include <stdio.h>
#include <stdint.h>
#include <pigpio.h>
#include "sps30_i2c_lib.h"
static int handler = -1;

uint8_t CalcCrc(char* data) {

    uint8_t crc = 0xFF;

    for(int i = 0; i < 2; i++) {
        crc ^= data[i];
        for(uint8_t bit = 8; bit > 0; --bit) {
            if(crc & 0x80) {
                crc = (crc << 1) ^ 0x31u;
            } else {
                crc = (crc << 1);
            }
        }
    }

    return crc;
};

int start(unsigned bus, unsigned addr, unsigned flag) {

    if (gpioInitialise() < 0)
    {
        printf("pigpio initialisation failed\n");
        return 0;
    } else {
        printf("pigpio initialized ok\n");
    }

    handler = i2cOpen(bus, addr, flag);

    if (handler < 0) {
        printf("Open failed\n");
        return 0;
    }

    return 1;
};

int end() {

    if (i2cClose(handler) != 0) {
        printf("Close failed\n");
        return 0;
    }

    return 1;
};

uint8_t start_measure() {

    char arr[5];
    arr[0] = 0;
    arr[1] = 16;

    char arrCheck[2];
    arrCheck[0] = 0x03;
    arrCheck[1] = 0x00;

    uint8_t check;
    check = CalcCrc(arrCheck);

    arr[2] = arrCheck[0];
    arr[3] = arrCheck[1];
    arr[4] = check;

    if (i2cWriteDevice(handler, arr, 5) < 0) {
        printf("Start failed\n");
        return 0;
    }

    return 1;
};

uint8_t stop_measure() {

    char arr[2];
    arr[0] = 0x01;
    arr[1] = 0x04;

    if (i2cWriteDevice(handler, arr, 2) < 0) {
        printf("Stop failed\n");
        return 0;
    }

    return 1;
};

uint8_t device_reset() {

    char arr[2];
    arr[0] = 0xD3;
    arr[1] = 0x04;

    if (i2cWriteDevice(handler, arr, 2) < 0) {
        printf("Reset failed\n");
        return 0;
    }

    return 1;
};

uint8_t fan_clean() {

    char arr[2];
    arr[0] = 0x56;
    arr[1] = 0x07;

    if (i2cWriteDevice(handler, arr, 2) < 0) {
        printf("Fan clean failed\n");
        return 0;
    }

    return 1;
};

uint8_t get_interval(char* buf) {

    char arr[2];
    arr[0] = 0x80;
    arr[1] = 0x04;

    if (i2cWriteDevice(handler, arr, 2) < 0) {
        printf("Get Interval write failed\n");
        return 0;
    }

    if (i2cReadDevice(handler, buf, 6) < 0) {
        printf("Get interval read failed\n");
        return 0;
    }

    return 1;
};

uint8_t set_interval(uint32_t sec) {
    
    uint8_t msb1 = (sec >> 24);
    uint8_t msb2 = (sec >> 16);
    uint8_t lsb1 = (sec >> 8);
    uint8_t lsb2 = (sec);

    uint8_t check_msb;
    uint8_t check_lsb;
    char msb[2];
    char lsb[2];
    char arr[8];
    arr[0] = 0x80;
    arr[1] = 0x04;

    msb[0] = msb1;
    msb[1] = msb2;
    lsb[0] = lsb1;
    lsb[1] = lsb2;

    check_msb = CalcCrc(msb);
    check_lsb = CalcCrc(lsb);
    arr[2] = msb[0];
    arr[3] = msb[1];
    arr[4] = check_msb;
    arr[5] = lsb[0];
    arr[6] = lsb[1];
    arr[7] = check_lsb;

    if (i2cWriteDevice(handler, arr, 8) < 0) {
        printf("Set interval failed\n");
        return 0;
    }

    return 1;
};

uint8_t read_value(char* buf) {

    char arr[2];
    arr[0] = 0x03;
    arr[1] = 0x00;

    if (i2cWriteDevice(handler, arr,  2) < 0) {
        printf("Read value write failed\n");
        return 0;
    }

    if (i2cReadDevice(handler, buf, 60) < 0) {
        printf("Read value read failed %d\n", sizeof(buf));
        return 0;
    }

    return 1;
};

uint8_t read_flag(char* buf) {

    char arr[2];
    arr[0] = 0x02;
    arr[1] = 0x02;

    if (i2cWriteDevice(handler, arr, 2) < 0) {
        printf("Read flag write failed\n");
        return 0;
    }

    if (i2cReadDevice(handler, buf, 3) < 0) {
        printf("Read flag read failed\n");
        return 0;
    }
    
    return 1;
};

