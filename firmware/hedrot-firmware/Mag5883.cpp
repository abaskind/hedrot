// Mag5883.cpp: Mag5883 I2C device methods for both Magnetometers used in gy-85 boards:
// . Honeywell HMC5883L
// . QMC5883L
// part of code derived from [DFRobot](http://www.dfrobot.com), 2017
// part of code derived from I2Cdev library

#include "Mag5883.h"

bool Mag5883::initialize()
{
  int retry;
  retry = 5;
  
  while(retry--){
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
    if ((readByte(HMC5883L_ADDRESS, HMC5883L_REG_IDENT_A) != 0x48)
    || (readByte(HMC5883L_ADDRESS, HMC5883L_REG_IDENT_B) != 0x34)
    || (readByte(HMC5883L_ADDRESS, HMC5883L_REG_IDENT_C) != 0x33)){
      return false;
    }

	setMeasurementBias(HMC5883L_BIAS_NORMAL);
    setRange(HMC5883L_RANGE_0_88G);
    setMode(HMC5883L_MODE_SINGLE);
    setDataRate(HMC5883L_DATARATE_75HZ);
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
      writeByte(QMC5883_ADDRESS, QMC5883_REG_IDENT_B, 0X01);
      writeByte(QMC5883_ADDRESS, QMC5883_REG_IDENT_C, 0X40);
      writeByte(QMC5883_ADDRESS, QMC5883_REG_IDENT_D, 0X01);
      writeByte(QMC5883_ADDRESS, QMC5883_REG_CONFIG_1, 0X1D);
      
      if ((readByte(QMC5883_ADDRESS, QMC5883_REG_IDENT_B) != 0x01)
      || (readByte(QMC5883_ADDRESS, QMC5883_REG_IDENT_C) != 0x40)
      || (readByte(QMC5883_ADDRESS, QMC5883_REG_IDENT_D) != 0x01)){
        return false;
      }
      setRange(QMC5883_RANGE_8G);
      setMode(QMC5883_MODE_CONTINUOUS);
      setDataRate(QMC5883_DATARATE_200HZ);
      setSampleAveraging(QMC5883_SAMPLES_1);

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
    return ((readByte(HMC5883L_ADDRESS, HMC5883L_REG_IDENT_A) == 0x48)
            && (readByte(HMC5883L_ADDRESS, HMC5883L_REG_IDENT_B) == 0x34)
            && (readByte(HMC5883L_ADDRESS, HMC5883L_REG_IDENT_C) == 0x33));
  } else {
    return ((readByte(QMC5883_ADDRESS, QMC5883_REG_IDENT_B) == 0x01)
            && (readByte(QMC5883_ADDRESS, QMC5883_REG_IDENT_C) == 0x40)
            && (readByte(QMC5883_ADDRESS, QMC5883_REG_IDENT_D) == 0x01));
  }
}

/** Get measurement bias value.
 *  This function has no effect sif QMC5883
@return if HMC5883L: Current bias value (0-2 for normal/positive/negative respectively). If QMC5883 (0)
 */
uint8_t Mag5883::getMeasurementBias() {
    if(isHMC_){
      return readBits(HMC5883L_ADDRESS, HMC5883L_REG_CONFIG_A, HMC5883L_CRA_BIAS_BIT, HMC5883L_CRA_BIAS_LENGTH);
    } else {
      return 0; // no effect
    }
}

/** Set measurement bias value.
 *  This function has no effect if QMC5883
@@param bias if HMC5883L: New bias value (0-2 for normal/positive/negative respectively)
 */
void Mag5883::setMeasurementBias(uint8_t bias) {
    if(isHMC_){
      writeBits(HMC5883L_ADDRESS, HMC5883L_REG_CONFIG_A, HMC5883L_CRA_BIAS_BIT, HMC5883L_CRA_BIAS_LENGTH, bias);
    } else {
      //do nothing
    }
}

/** Get number of samples averaged per measurement.
 * @return Current samples averaged per measurement (0-3 for 1/2/4/8 respectively)
 */
uint8_t Mag5883::getSampleAveraging() {
    if(isHMC_){
      return readBits(HMC5883L_ADDRESS, HMC5883L_REG_CONFIG_A, HMC5883L_CRA_AVERAGE_BIT, HMC5883L_CRA_AVERAGE_LENGTH);
    } else {
      return readBits(QMC5883_ADDRESS, QMC5883_REG_CONFIG_1, QMC5883_CRA_AVERAGE_BIT, QMC5883_CRA_AVERAGE_LENGTH);
    }
}
/** Set number of samples averaged per measurement.
 * @param averaging New samples averaged per measurement setting(0-3 for 1/2/4/8 respectively)
 */
void Mag5883::setSampleAveraging(uint8_t averaging) {
    if(isHMC_){
      writeBits(HMC5883L_ADDRESS, HMC5883L_REG_CONFIG_A, HMC5883L_CRA_AVERAGE_BIT, HMC5883L_CRA_AVERAGE_LENGTH, averaging);
    } else {
      writeBits(QMC5883_ADDRESS, QMC5883_REG_CONFIG_1, QMC5883_CRA_AVERAGE_BIT, QMC5883_CRA_AVERAGE_LENGTH, averaging);
    }
}

/** get magnetic field gain value
***** FOR HMC5883L *******
 * The table below shows nominal gain settings. Use the "Gain" column to convert
 * counts to Gauss. Choose a lower gain value (higher GN#) when total field
 * strength causes overflow in one of the data output registers (saturation).
 * The data output range for all settings is 0xF800-0x07FF (-2048 - 2047).
 *
 * Value | Field Range | Gain (LSB/Gauss)
 * ------+-------------+-----------------
 * 0     | +/- 0.88 Ga | 1370
 * 1     | +/- 1.3 Ga  | 1090 (Default)
 * 2     | +/- 1.9 Ga  | 820
 * 3     | +/- 2.5 Ga  | 660
 * 4     | +/- 4.0 Ga  | 440
 * 5     | +/- 4.7 Ga  | 390
 * 6     | +/- 5.6 Ga  | 330
 * 7     | +/- 8.1 Ga  | 230
 *
***** FOR QMC5883 *******
 *
 * Value | Recommended Range
 * ------+------------
 * 0     | +/- 2 G
 * 1     | +/- 8 G
 * 
 * @return Current magnetic field gain value
 */
uint8_t Mag5883::getRange() {
    if(isHMC_){
      return readBits(HMC5883L_ADDRESS, HMC5883L_REG_CONFIG_B, HMC5883L_CRB_RANGE_BIT, HMC5883L_CRB_RANGE_LENGTH);
    } else {
      return readBits(QMC5883_ADDRESS, QMC5883_REG_CONFIG_1, QMC5883_CRB_RANGE_BIT, QMC5883_CRB_RANGE_LENGTH);
    }
}

/** Set magnetic field gain value
 */
void Mag5883::setRange(uint8_t gain) {
    if(isHMC_){
      // use this method to guarantee that bits 4-0 are set to zero, which is a
      // requirement specified in the datasheet; it's actually more efficient than
      // using the I2Cdev.writeBits method
      writeByte(HMC5883L_ADDRESS, HMC5883L_REG_CONFIG_B, gain << 5);
    } else {
      writeBits(QMC5883_ADDRESS, QMC5883_REG_CONFIG_1, QMC5883_CRB_RANGE_BIT, QMC5883_CRB_RANGE_LENGTH, gain);
    }
}

/** Get measurement mode.
***** FOR HMC5883L *******
 * In continuous-measurement mode, the device continuously performs measurements
 * and places the result in the data register. RDY goes high when new data is
 * placed in all three registers. After a power-on or a write to the mode or
 * configuration register, the first measurement set is available from all three
 * data output registers after a period of 2/fDO and subsequent measurements are
 * available at a frequency of fDO, where fDO is the frequency of data output.
 *
 * When single-measurement mode (default) is selected, device performs a single
 * measurement, sets RDY high and returned to idle mode. Mode register returns
 * to idle mode bit values. The measurement remains in the data output register
 * and RDY remains high until the data output register is read or another
 * measurement is performed.
 *
***** FOR QMC5883 *******
 * Measurement mode is forced to continous, rate is defined by user 
 */
 
uint8_t Mag5883::getMode() {
    if(isHMC_){
      return readBits(HMC5883L_ADDRESS, HMC5883L_REG_MODE, HMC5883L_MODEREG_BIT, HMC5883L_MODEREG_LENGTH);
    } else {
      return 1; //measurement mode forced to continuous
    }
}

/** Set measurement mode (forced to continous for QMC5883)
 */
void Mag5883::setMode(uint8_t newMode) {
    if(isHMC_){
      // use this method to guarantee that bits 7-2 are set to zero, which is a
      // requirement specified in the datasheet; it's actually more efficient than
      // using the I2Cdev.writeBits method
      writeByte(HMC5883L_ADDRESS, HMC5883L_REG_MODE, newMode << (HMC5883L_MODEREG_BIT - HMC5883L_MODEREG_LENGTH + 1));
      mode = newMode; // track to tell if we have to clear bit 7 after a read
    } else {
        newMode = 1;// force to 1 (continuous)
        mode = newMode;

        writeBits(QMC5883_ADDRESS, QMC5883_REG_CONFIG_1, QMC5883_MODEREG_BIT, QMC5883_MODEREG_LENGTH, newMode);
    }
}

/** Get data output rate value.
***** FOR HMC5883L *******
 * The Table below shows all selectable output rates in continuous measurement
 * mode. All three channels shall be measured within a given output rate. Other
 * output rates with maximum rate of 160 Hz can be achieved by monitoring DRDY
 * interrupt pin in single measurement mode.
 *
 * Value | Typical Data Output Rate (Hz)
 * ------+------------------------------
 * 0     | 0.75
 * 1     | 1.5
 * 2     | 3
 * 3     | 7.5
 * 4     | 15 (Default)
 * 5     | 30
 * 6     | 75
 * 7     | Not used
 *
 *
***** FOR QMC5883 *******
 * 
 *  * Value | Typical Data Output Rate (Hz)
 * ------+------------------------------
 * 0     | 10
 * 1     | 50
 * 2     | 100
 * 3     | 200
 * 
 * @return Current rate of data output to registers
 */
uint8_t Mag5883::getDataRate() {
  return 1;
    if(isHMC_){
      return readBits(HMC5883L_ADDRESS, HMC5883L_REG_CONFIG_A, HMC5883L_CRA_RATE_BIT, HMC5883L_CRA_RATE_LENGTH);
    } else {
      return readBits(QMC5883_ADDRESS, QMC5883_REG_CONFIG_1, QMC5883_CRA_RATE_BIT, QMC5883_CRA_RATE_LENGTH);
    }
}

/** Set data output rate value.
 */
void Mag5883::setDataRate(uint8_t rate) {
    if(isHMC_){
      writeBits(HMC5883L_ADDRESS, HMC5883L_REG_CONFIG_A, HMC5883L_CRA_RATE_BIT, HMC5883L_CRA_RATE_LENGTH, rate);
    } else {
      writeBits(QMC5883_ADDRESS, QMC5883_REG_CONFIG_1, QMC5883_CRA_RATE_BIT, QMC5883_CRA_RATE_LENGTH, rate);
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
    } else {
      readBytes(QMC5883_ADDRESS, QMC5883_REG_OUT_X_L, 6, buffer);
      //if (mode == HMC5883L_MODE_SINGLE) writeByte(devAddr, HMC5883L_RA_MODE, HMC5883L_MODE_SINGLE << (HMC5883L_MODEREG_BIT - HMC5883L_MODEREG_LENGTH + 1));
      *x = (((int16_t)buffer[1]) << 8) | buffer[0]; // careful LSB before MSB
      *z = (((int16_t)buffer[5]) << 8) | buffer[4]; // careful LSB before MSB
      *y = (((int16_t)buffer[3]) << 8) | buffer[2]; // careful LSB before MSB
    }
}


