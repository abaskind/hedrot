// I2Ccom: I2C communication with the I2c_t3 library

#include "I2Ccom.h"

// pas ultra efficace car on fait appel à deux autres fonctions
void writeBit(uint8_t address, uint8_t subAddress, uint8_t bitnumber, uint8_t data) {
    uint8_t b = readByte(address, subAddress);
    b = (data != 0) ? (b | (1 << bitnumber)) : (b & ~(1 << bitnumber));
    writeByte(address, subAddress, b);
}

void writeBits(uint8_t address, uint8_t subAddress, uint8_t bitStart, uint8_t length, uint8_t data) {
    //      010 value to write
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    // 00011100 mask byte
    // 10101111 original value (sample)
    // 10100011 original & ~mask
    // 10101011 masked | value
    uint8_t b = readByte(address, subAddress);
    uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
    data <<= (bitStart - length + 1); // shift data into correct position
    data &= mask; // zero all non-important bits in data
    b &= ~(mask); // zero all important bits in existing byte
    b |= data; // combine data with existing byte
    writeByte(address, subAddress, b);
}

void writeByte(uint8_t address, uint8_t subAddress, uint8_t data) {
  Wire.beginTransmission(address); // Initialize the Tx buffer
  Wire.write(subAddress); // Put slave register address in Tx buffer
  Wire.write(data); // Put data in Tx buffer
  Wire.endTransmission(); // Send the Tx buffer
}

bool readBit(uint8_t address, uint8_t subAddress, uint8_t bitnumber) {
  uint8_t data; // `data` will store the register data (whole byte)
  Wire.beginTransmission(address); // Initialize the Tx buffer
  Wire.write(subAddress); // Put slave register address in Tx buffer
  Wire.endTransmission(I2C_NOSTOP); // Send the Tx buffer, but send a restart to keep connection alive
  // Wire.endTransmission(false); // Send the Tx buffer, but send a restart to keep connection alive
  // Wire.requestFrom(address, 1); // Read one byte from slave register address
  Wire.requestFrom(address, (size_t) 1); // Read one byte from slave register address
  data = Wire.read(); // Fill Rx buffer with result
  return data & (1 << bitnumber); // Return data read from slave register
}

uint8_t readBits(uint8_t address, uint8_t subAddress, uint8_t bitStart, uint8_t length) {
    // 01101001 read byte
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    //    010   masked
    //   -> 010 shifted
    uint8_t b;
    b = readByte(address, subAddress);
    uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
    b &= mask;
    b >>= (bitStart - length + 1);

    return b;
}

uint8_t readByte(uint8_t address, uint8_t subAddress) {
  uint8_t data; // `data` will store the register data
  Wire.beginTransmission(address); // Initialize the Tx buffer
  Wire.write(subAddress); // Put slave register address in Tx buffer
  Wire.endTransmission(I2C_NOSTOP); // Send the Tx buffer, but send a restart to keep connection alive
  // Wire.endTransmission(false); // Send the Tx buffer, but send a restart to keep connection alive
  // Wire.requestFrom(address, 1); // Read one byte from slave register address
  Wire.requestFrom(address, (size_t) 1); // Read one byte from slave register address
  data = Wire.read(); // Fill Rx buffer with result
  return data; // Return data read from slave register
}

int8_t readBytes(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t * dest) {
  Wire.beginTransmission(address); // Initialize the Tx buffer
  Wire.write(subAddress); // Put slave register address in Tx buffer
  Wire.endTransmission(I2C_NOSTOP); // Send the Tx buffer, but send a restart to keep connection alive
  // Wire.endTransmission(false); // Send the Tx buffer, but send a restart to keep connection alive
  uint8_t i = 0;
  // Wire.requestFrom(address, count); // Read bytes from slave register address
  Wire.requestFrom(address, (size_t) count); // Read bytes from slave register address
  while (Wire.available()) {
  dest[i++] = Wire.read(); } // Put read results in the Rx buffer
  return i;
}

