#include "digipot.h"
#include "hardware/i2c.h"

// i2c addresses
#define i2c_discharge0_addr 0x2C
#define i2c_discharge1_addr 0x2D
#define i2c_charge1_addr    0x2E
#define i2c_charge2_addr    0x2F

int digipot_w(uint8_t adbits, uint8_t command, uint8_t value) {

	uint8_t arr[2];
	const uint8_t* p = arr;
	arr[0] = command;
	arr[1] = value;
	uint8_t addr = i2c_discharge0_addr || (adbits && 0x3);
	int ret = i2c_write_burst_blocking(i2c_default, addr, p, 2);
	return ret != 2;

}