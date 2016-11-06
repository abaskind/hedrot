// I2Ccom: I2C communication with the I2c_t3 library


#ifndef _I2CCOM_H_
#define _I2CCOM_H_

#include "i2c_t3.h"

void writeBit(uint8_t address, uint8_t subAddress, uint8_t bitnumber, uint8_t data);
void writeBits(uint8_t address, uint8_t subAddress, uint8_t bitStart, uint8_t length, uint8_t data);
void writeByte(uint8_t address, uint8_t subAddress, uint8_t data);
bool readBit(uint8_t address, uint8_t subAddress, uint8_t bitnumber);
uint8_t readBits(uint8_t address, uint8_t subAddress, uint8_t bitStart, uint8_t length);
uint8_t readByte(uint8_t address, uint8_t subAddress);
int8_t readBytes(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t * dest);


#endif /* _I2CCOM_H_ */
