/*!
 * @file Mag5883.cpp
 * @brief Compatible with QMC5883 and QMC5883
 * @n 3-Axis Digital Compass IC
 *
 * @copyright	[DFRobot](http://www.dfrobot.com), 2017
 * @copyright	GNU Lesser General Public License
 *
 * @author [dexian.huang](952838602@qq.com)
 * @version  V1.0
 * @date  2017-7-3
 */

 /* A FAIRE
  . unifier méthodes lecture/écriture
  . documenter toutes les options avec valeurs binaires
  . compléter pour QMC
  . adapter changeRTMagCalTimeSettings surtout mais tout l'objet Max en général
  */

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "Mag5883.h"

#include "I2Ccom.h" // mélange sale de I2Ccom et des méthodes de DFRobot



bool Mag5883::initialize()
{
  int retry;
  retry = 5;
  
  while(retry--){
    Wire.begin();
    Wire.beginTransmission(HMC5883L_ADDRESS);
    isHMC_ = (0 == Wire.endTransmission());
    if(isHMC_){
      break;
    }
    delay(20);
  }
  //Serial.print("isHMC_= ");
  //Serial.println(isHMC_);

  if(isHMC_){
    if ((fastRegister8(HMC5883L_REG_IDENT_A) != 0x48)
    || (fastRegister8(HMC5883L_REG_IDENT_B) != 0x34)
    || (fastRegister8(HMC5883L_REG_IDENT_C) != 0x33)){
      return false;
    }

    setGain(HMC5883L_RANGE_0_88GA);
    setMode(HMC5883L_CONTINOUS);
    setDataRate(HMC5883L_DATARATE_15HZ);
    setSampleAveraging(HMC5883L_SAMPLES_1);

    return true;
  }else{
    retry = 5;
    while(retry--){
      Wire.begin();
      Wire.beginTransmission(QMC5883_ADDRESS);
      isQMC_ = (0 == Wire.endTransmission());
      if(isHMC_){
        break;
      }
      delay(20);
    }
    //Serial.print("isQMC_= ");
    //Serial.println(isQMC_);
    if(isQMC_){
      writeRegister8(QMC5883_REG_IDENT_B,0X01);
      writeRegister8(QMC5883_REG_IDENT_C,0X40);
      writeRegister8(QMC5883_REG_IDENT_D,0X01);
      writeRegister8(QMC5883_REG_CONFIG_1,0X1D);
      if ((fastRegister8(QMC5883_REG_IDENT_B) != 0x01)
      || (fastRegister8(QMC5883_REG_IDENT_C) != 0x40)
      || (fastRegister8(QMC5883_REG_IDENT_D) != 0x01)){
        return false;
      }
      setGain(QMC5883_RANGE_8GA);
      setMode(QMC5883_CONTINOUS);
      setDataRate(QMC5883_DATARATE_200HZ);
      setSampleAveraging(QMC5883_SAMPLES_8);

      return true;
    }
  }
  return false;
}

/** Verify the I2C connection.
 * Make sure the device is connected and responds as expected.
 * @return True if connection is valid, false otherwise
 */
bool Mag5883::testConnection() {
  if(isHMC_){
    return ((fastRegister8(HMC5883L_REG_IDENT_A) == 0x48)
            && (fastRegister8(HMC5883L_REG_IDENT_B) == 0x34)
            && (fastRegister8(HMC5883L_REG_IDENT_C) == 0x33));
  } else {
    return ((fastRegister8(QMC5883_REG_IDENT_B) == 0x01)
            && (fastRegister8(QMC5883_REG_IDENT_C) == 0x40)
            && (fastRegister8(QMC5883_REG_IDENT_D) == 0x01));
  }
}

/** Get measurement bias value.
dirty patch for QMC5883, ok for HMC5883L
 */
uint8_t Mag5883::getMeasurementBias() {
    if(isHMC_){
      return readBits(HMC5883L_ADDRESS, HMC5883L_REG_CONFIG_A, HMC5883L_CRA_BIAS_BIT, HMC5883L_CRA_BIAS_LENGTH);
    } else {
      return 0; //dirty
    }
}
/** Set measurement bias value.
dirty patch for QMC5883, ok for HMC5883L
 */
void Mag5883::setMeasurementBias(uint8_t bias) {
    if(isHMC_){
      writeBits(HMC5883L_ADDRESS, HMC5883L_REG_CONFIG_A, HMC5883L_CRA_BIAS_BIT, HMC5883L_CRA_BIAS_LENGTH, bias);
    } else {
      //dirty
    }
}

/** get magnetic field gain value ("range" in DFRobot).
dirty patch for QMC5883, ok for HMC5883L
 */
uint8_t Mag5883::getGain() {
    if(isHMC_){
      return readBits(HMC5883L_ADDRESS, HMC5883L_REG_CONFIG_B, HMC5883L_CRB_GAIN_BIT, HMC5883L_CRB_GAIN_LENGTH);
    } else {
      return 0; //8GA, fixe
    }
}

/** Set magnetic field gain value ("range" in DFRobot).
dirty patch for QMC5883, ok for HMC5883L
 */
void Mag5883::setGain(uint8_t gain) {
    if(isHMC_){
      // use this method to guarantee that bits 4-0 are set to zero, which is a
      // requirement specified in the datasheet; it's actually more efficient than
      // using the I2Cdev.writeBits method
      writeByte(HMC5883L_ADDRESS, HMC5883L_REG_CONFIG_B, gain << 5);
    } else {
      gain = 1;//set to 8GA (dirty)
      writeByte(QMC5883_ADDRESS, QMC5883_REG_CONFIG_2, gain << 4);
    }
}

/** Get number of samples averaged per measurement.
dirty patch for QMC5883, ok for HMC5883L
 */
uint8_t Mag5883::getSampleAveraging() {
    if(isHMC_){
      return readBits(HMC5883L_ADDRESS, HMC5883L_REG_CONFIG_A, HMC5883L_CRA_AVERAGE_BIT, HMC5883L_CRA_AVERAGE_LENGTH);
    } else {
      return 0;
    }
}
/** Set number of samples averaged per measurement.
dirty patch for QMC5883, ok for HMC5883L
 */
void Mag5883::setSampleAveraging(uint8_t averaging) {
    if(isHMC_){
      writeBits(HMC5883L_ADDRESS, HMC5883L_REG_CONFIG_A, HMC5883L_CRA_AVERAGE_BIT, HMC5883L_CRA_AVERAGE_LENGTH, averaging);
    } else {
      averaging = 0;// set to 1 sample (dirty)

      uint8_t value;
      value = readRegister8(QMC5883_REG_CONFIG_1);
      value &= 0x3f;
      value |= (averaging << 6);
      writeRegister8(QMC5883_REG_CONFIG_1, value);
    }
}

/** Get data output rate value.
Mag5883
 */
uint8_t Mag5883::getDataRate() {
  return 1;
    if(isHMC_){
      return readBits(HMC5883L_ADDRESS, HMC5883L_REG_CONFIG_A, HMC5883L_CRA_RATE_BIT, HMC5883L_CRA_RATE_LENGTH);
    } else {
      return 0b11; // dirty
    }
}

/** Set data output rate value.
dirty patch for QMC5883, ok for HMC5883L
 */
void Mag5883::setDataRate(uint8_t rate) {
    if(isHMC_){
      writeBits(HMC5883L_ADDRESS, HMC5883L_REG_CONFIG_A, HMC5883L_CRA_RATE_BIT, HMC5883L_CRA_RATE_LENGTH, rate);
    } else {
      uint8_t value;
      rate = 0b11;//set to 200Hz (dirty)
      value = readRegister8(QMC5883_REG_CONFIG_1);
      value &= 0xf3;
      value |= (rate << 2);
    }
}

/** Get measurement mode.
dirty patch for QMC5883, ok for HMC5883L
 */
 
uint8_t Mag5883::getMode() {
    if(isHMC_){
      return readBits(HMC5883L_ADDRESS, HMC5883L_REG_MODE, HMC5883L_MODEREG_BIT, HMC5883L_MODEREG_LENGTH);
    } else {
      return 0; //dirty
    }
}

/** Set measurement mode.
dirty patch for QMC5883, ok for HMC5883L
 */
void Mag5883::setMode(uint8_t newMode) {
    if(isHMC_){
      // use this method to guarantee that bits 7-2 are set to zero, which is a
      // requirement specified in the datasheet; it's actually more efficient than
      // using the I2Cdev.writeBits method
      writeByte(HMC5883L_ADDRESS, HMC5883L_REG_MODE, newMode << (HMC5883L_MODEREG_BIT - HMC5883L_MODEREG_LENGTH + 1));
      mode = newMode; // track to tell if we have to clear bit 7 after a read
    } else {
        newMode = 1;// set to 1 (continuous) (dirty)
        uint8_t value;
        value = readRegister8(QMC5883_REG_CONFIG_1);
        value &= 0xfc;
        value |= newMode;
    
        writeRegister8(QMC5883_REG_CONFIG_1, value);
    }
}

/** Get 3-axis heading measurements.
 * In the event the ADC reading overflows or underflows for the given channel,
 * or if there is a math overflow during the bias measurement, this data
 * register will contain the value -4096. This register value will clear when
 * after the next valid measurement is made. Note that this method automatically
 * clears the appropriate bit in the MODE register if Single mode is active.
 * @param x 16-bit signed integer container for X-axis heading
 * @param y 16-bit signed integer container for Y-axis heading
 * @param z 16-bit signed integer container for Z-axis heading
 * @see HMC5883L_RA_DATAX_H
 */
void Mag5883::getHeading(int16_t *x, int16_t *y, int16_t *z) {
    if(isHMC_){
      readBytes(HMC5883L_ADDRESS, HMC5883L_REG_OUT_X_M, 6, buffer);
      if (mode == HMC5883L_MODE_SINGLE) writeByte(HMC5883L_ADDRESS, HMC5883L_REG_MODE, HMC5883L_MODE_SINGLE << (HMC5883L_MODEREG_BIT - HMC5883L_MODEREG_LENGTH + 1));
      *x = (((int16_t)buffer[0]) << 8) | buffer[1];
      *y = (((int16_t)buffer[4]) << 8) | buffer[5];
      *z = (((int16_t)buffer[2]) << 8) | buffer[3];
    } else { // dirty
      readBytes(QMC5883_ADDRESS, QMC5883_REG_OUT_X_L, 6, buffer);
      //if (mode == HMC5883L_MODE_SINGLE) writeByte(devAddr, HMC5883L_RA_MODE, HMC5883L_MODE_SINGLE << (HMC5883L_MODEREG_BIT - HMC5883L_MODEREG_LENGTH + 1));
      *x = (((int16_t)buffer[1]) << 8) | buffer[0]; // careful LSB before MSB
      *z = (((int16_t)buffer[5]) << 8) | buffer[4]; // careful LSB before MSB
      *y = (((int16_t)buffer[3]) << 8) | buffer[2]; // careful LSB before MSB
    }
}

/*Vector Mag5883::readRaw(void)
{
  int range = 10;
  float Xsum = 0.0;
  float Ysum = 0.0;
  float Zsum = 0.0;
  if(isHMC_){
    while(range--){
      v.XAxis = readRegister16(HMC5883L_REG_OUT_X_M);
      v.YAxis = readRegister16(HMC5883L_REG_OUT_Y_M);
      v.ZAxis = readRegister16(HMC5883L_REG_OUT_Z_M);
      calibrate();
      Xsum += v.XAxis;
      Ysum += v.YAxis;
      Zsum += v.ZAxis;
    }
    v.XAxis = Xsum/range;
    v.YAxis = Ysum/range;
    v.ZAxis = Zsum/range;
    if(firstRun){
      initMinMax();
      firstRun = false;
    }
  }else if(isQMC_){
    while (range--){
      v.XAxis = readRegister16(QMC5883_REG_OUT_X_M);
      v.YAxis = readRegister16(QMC5883_REG_OUT_Y_M);
      v.ZAxis = readRegister16(QMC5883_REG_OUT_Z_M);
      calibrate();
      Xsum += v.XAxis;
      Ysum += v.YAxis;
      Zsum += v.ZAxis;
    }
    v.XAxis = Xsum/range;
    v.YAxis = Ysum/range;
    v.ZAxis = Zsum/range;
    if(firstRun){
      initMinMax();
      firstRun = false;
    }
  }
  return v;
}
void Mag5883::calibrate()
{
  if(v.XAxis < minX ) minX = v.XAxis;
  if(v.XAxis > maxX ) maxX = v.XAxis;
  if(v.YAxis < minY ) minY = v.YAxis;
  if(v.YAxis > maxY ) maxY = v.YAxis;
  if(v.ZAxis < minZ ) minZ = v.ZAxis;
  if(v.ZAxis > maxZ ) maxZ = v.ZAxis;
}
void Mag5883::initMinMax()
{
  minX = v.XAxis;
  maxX = v.XAxis;
  minY = v.YAxis;
  maxY = v.YAxis;
  minZ = v.ZAxis;
  maxZ = v.ZAxis;
}*/


/*void Mag5883::setRange(QMC5883_range_t range)
{
  if(isHMC_){
    switch(range){
    case HMC5883L_RANGE_0_88GA:
      mgPerDigit = 0.073f;
      break;

    case HMC5883L_RANGE_1_3GA:
      mgPerDigit = 0.92f;
      break;

    case HMC5883L_RANGE_1_9GA:
      mgPerDigit = 1.22f;
      break;

    case HMC5883L_RANGE_2_5GA:
      mgPerDigit = 1.52f;
      break;

    case HMC5883L_RANGE_4GA:
      mgPerDigit = 2.27f;
      break;

    case HMC5883L_RANGE_4_7GA:
      mgPerDigit = 2.56f;
      break;

    case HMC5883L_RANGE_5_6GA:
      mgPerDigit = 3.03f;
      break;

    case HMC5883L_RANGE_8_1GA:
      mgPerDigit = 4.35f;
      break;

    default:
      break;
    }

    writeRegister8(HMC5883L_REG_CONFIG_B, range << 5);
  }else if(isQMC_){
    switch(range)
    {
    case QMC5883_RANGE_2GA:
      mgPerDigit = 1.22f;
      break;
    case QMC5883_RANGE_8GA:
      mgPerDigit = 4.35f;
      break;
    default:
      break;
    }

    writeRegister8(QMC5883_REG_CONFIG_2, range << 4);
  }
}

QMC5883_range_t Mag5883::getRange(void)
{
  if(isHMC_){
    return (QMC5883_range_t)((readRegister8(HMC5883L_REG_CONFIG_B) >> 5));
  }else if(isQMC_){
    return (QMC5883_range_t)((readRegister8(QMC5883_REG_CONFIG_2) >> 4));
  }
  return QMC5883_RANGE_8GA;
}*/

/*
void Mag5883::setMeasurementMode(QMC5883_mode_t mode)
{
  uint8_t value;
  if(isHMC_){
    value = readRegister8(HMC5883L_REG_MODE);
    value &= 0b11111100;
    value |= mode;

    writeRegister8(HMC5883L_REG_MODE, value);
  }else if(isQMC_){
    value = readRegister8(QMC5883_REG_CONFIG_1);
    value &= 0xfc;
    value |= mode;

    writeRegister8(QMC5883_REG_CONFIG_1, value);
  }
}

QMC5883_mode_t Mag5883::getMeasurementMode(void)
{
  uint8_t value=0;
  if(isHMC_){
    value = readRegister8(HMC5883L_REG_MODE);
  }else if(isQMC_){
    value = readRegister8(QMC5883_REG_CONFIG_1); 
  }
  value &= 0b00000011;  
  return (QMC5883_mode_t)value;
}*/

/*void Mag5883::setDataRate(QMC5883_dataRate_t dataRate)
{
  uint8_t value;
  if(isHMC_){
    value = readRegister8(HMC5883L_REG_CONFIG_A);
    value &= 0b11100011;
    value |= (dataRate << 2);

    writeRegister8(HMC5883L_REG_CONFIG_A, value);
  }else if(isQMC_){
    value = readRegister8(QMC5883_REG_CONFIG_1);
    value &= 0xf3;
    value |= (dataRate << 2);

    writeRegister8(QMC5883_REG_CONFIG_1, value);
  }
}

QMC5883_dataRate_t Mag5883::getDataRate(void)
{
  uint8_t value=0;
  if(isHMC_){
    value = readRegister8(HMC5883L_REG_CONFIG_A);
    value &= 0b00011100;
    value >>= 2;
  }else if(isQMC_){
    value = readRegister8(QMC5883_REG_CONFIG_1);
    value &= 0b00001100;
    value >>= 2;
  }
  return (QMC5883_dataRate_t)value;
}*/

/*void Mag5883::setSamples(QMC5883_samples_t samples)
{
  uint8_t value;
  if(isHMC_){
    value = readRegister8(HMC5883L_REG_CONFIG_A);
    value &= 0b10011111;
    value |= (samples << 5);
    writeRegister8(HMC5883L_REG_CONFIG_A, value);
  }else if(isQMC_){
    value = readRegister8(QMC5883_REG_CONFIG_1);
    value &= 0x3f;
    value |= (samples << 6);
    writeRegister8(QMC5883_REG_CONFIG_1, value);
  }
}

QMC5883_samples_t Mag5883::getSamples(void)
{
  uint8_t value=0;
  if(isHMC_){
    value = readRegister8(HMC5883L_REG_CONFIG_A);
    value &= 0b01100000;
    value >>= 5;
  }else if(isQMC_){
    value = readRegister8(QMC5883_REG_CONFIG_1);
    value &= 0x3f;
    value >>= 6;
  }
  return (QMC5883_samples_t)value;
}*/

// Write byte to register
void Mag5883::writeRegister8(uint8_t reg, uint8_t value)
{
  if(isHMC_){
    Wire.beginTransmission(HMC5883L_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(reg);
        Wire.write(value);
    #else
        Wire.send(reg);
        Wire.send(value);
    #endif
    Wire.endTransmission();
  }else if(isQMC_){
    Wire.beginTransmission(QMC5883_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(reg);
        Wire.write(value);
    #else
        Wire.send(reg);
        Wire.send(value);
    #endif
    Wire.endTransmission();
  }
}
// Read byte to register
uint8_t Mag5883::fastRegister8(uint8_t reg)
{
  uint8_t value=0;
  if(isHMC_){
    Wire.beginTransmission(HMC5883L_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(reg);
    #else
        Wire.send(reg);
    #endif
    Wire.endTransmission();

    Wire.requestFrom(HMC5883L_ADDRESS, 1);
    #if ARDUINO >= 100
        value = Wire.read();
    #else
        value = Wire.receive();
    #endif
    Wire.endTransmission();
  }else if(isQMC_){
    Wire.beginTransmission(QMC5883_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(reg);
    #else
        Wire.send(reg);
    #endif
    Wire.endTransmission();
    Wire.requestFrom(QMC5883_ADDRESS, 1);
    #if ARDUINO >= 100
        value = Wire.read();
    #else
        value = Wire.receive();
    #endif
    Wire.endTransmission();
  }
  return value;
}

// Read byte from register
uint8_t Mag5883::readRegister8(uint8_t reg)
{
  uint8_t value=0;
  if(isHMC_){
    Wire.beginTransmission(HMC5883L_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(reg);
    #else
        Wire.send(reg);
    #endif
    Wire.endTransmission();

    Wire.beginTransmission(HMC5883L_ADDRESS);
    Wire.requestFrom(HMC5883L_ADDRESS, 1);
    while(!Wire.available()) {};
    #if ARDUINO >= 100
        value = Wire.read();
    #else
        value = Wire.receive();
    #endif
    Wire.endTransmission();
  }else if(isQMC_){
    Wire.beginTransmission(QMC5883_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(reg);
    #else
        Wire.send(reg);
    #endif
    Wire.endTransmission();
    Wire.beginTransmission(QMC5883_ADDRESS);
    Wire.requestFrom(QMC5883_ADDRESS, 1);
    while(!Wire.available()) {};
    #if ARDUINO >= 100
        value = Wire.read();
    #else
        value = Wire.receive();
    #endif
    Wire.endTransmission();
  }
  return value;
}
// Read word from register
int16_t Mag5883::readRegister16(uint8_t reg)
{
  int16_t value=0;
  if(isHMC_){
    Wire.beginTransmission(HMC5883L_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(reg);
    #else
        Wire.send(reg);
    #endif
    Wire.endTransmission();
    Wire.beginTransmission(HMC5883L_ADDRESS);
    Wire.requestFrom(HMC5883L_ADDRESS, 2);
    while(!Wire.available()) {};
    #if ARDUINO >= 100
        uint8_t vha = Wire.read();
        uint8_t vla = Wire.read();
    #else
        uint8_t vha = Wire.receive();
        uint8_t vla = Wire.receive();
    #endif
    Wire.endTransmission();
    value = vha << 8 | vla;
  }else if(isQMC_){
    Wire.beginTransmission(QMC5883_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(reg);
    #else
        Wire.send(reg);
    #endif
    Wire.endTransmission();
    Wire.beginTransmission(QMC5883_ADDRESS);
    Wire.requestFrom(QMC5883_ADDRESS, 2);
    while(!Wire.available()) {};
    #if ARDUINO >= 100
        uint8_t vha = Wire.read();
        uint8_t vla = Wire.read();
    #else
        uint8_t vha = Wire.receive();
        uint8_t vla = Wire.receive();
    #endif
    Wire.endTransmission();
    value = vha << 8 | vla;
  }
  return value;
}

int Mag5883::getICType(void)
{
  if(isHMC_){
    return IC_HMC5883L;
  }else if(isQMC_){
    return IC_QMC5883;
  }else{
    return IC_NONE;
  }
}
