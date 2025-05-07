#include <stdint.h>

// i2c ad bits
#define i2c_discharge0_bits 0x00
#define i2c_discharge1_bits 0x01
#define i2c_charge1_bits    0x02
#define i2c_charge2_bits    0x03

int digipot_w(uint8_t adbits, uint8_t command, uint8_t value);
int digipot_wb(uint8_t adbits, uint8_t command, uint8_t value);