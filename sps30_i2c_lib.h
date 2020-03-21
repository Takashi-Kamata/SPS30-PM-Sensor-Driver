#ifndef sps30_i2c_lib
#define sps30_i2c_lib

uint8_t CalcCrc(char* data);
uint8_t dec_hex(char* hex, uint32_t dec);
int start(unsigned bus, unsigned addr, unsigned flag);
int end();
uint8_t start_measure();
uint8_t stop_measure();
uint8_t device_reset();
uint8_t fan_clean();
uint8_t get_interval(char* buf);
uint8_t set_interval(uint32_t sec);
uint8_t read_value(char* buf);
uint8_t read_flag(char* buf);

#endif