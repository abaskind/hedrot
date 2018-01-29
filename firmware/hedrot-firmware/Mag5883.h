// Mag5883.h: Mag5883 I2C device methods for both Magnetometers used in gy-85 boards:
// . Honeywell HMC5883L
// . QMC5883L
// part of code derived from [DFRobot](http://www.dfrobot.com), 2017
// part of code derived from I2Cdev library

#ifndef Mag5883_H
#define Mag5883_H

#include "I2Ccom.h"


#define HMC5883L_ADDRESS               0x1E
#define QMC5883_ADDRESS                0x0D


#define HMC5883L_REG_CONFIG_A         0x00
#define HMC5883L_REG_CONFIG_B         0x01
#define HMC5883L_REG_MODE             0x02
#define HMC5883L_REG_OUT_X_M          0x03
#define HMC5883L_REG_OUT_X_L          0x04
#define HMC5883L_REG_OUT_Z_M          0x05
#define HMC5883L_REG_OUT_Z_L          0x06
#define HMC5883L_REG_OUT_Y_M          0x07
#define HMC5883L_REG_OUT_Y_L          0x08
#define HMC5883L_REG_STATUS           0x09
#define HMC5883L_REG_IDENT_A          0x0A
#define HMC5883L_REG_IDENT_B          0x0B
#define HMC5883L_REG_IDENT_C          0x0C


#define QMC5883_REG_OUT_X_M          0x01
#define QMC5883_REG_OUT_X_L          0x00
#define QMC5883_REG_OUT_Z_M          0x05
#define QMC5883_REG_OUT_Z_L          0x04
#define QMC5883_REG_OUT_Y_M          0x03
#define QMC5883_REG_OUT_Y_L          0x02
#define QMC5883_REG_STATUS           0x06
#define QMC5883_REG_CONFIG_1         0x09
#define QMC5883_REG_CONFIG_2         0x0A
#define QMC5883_REG_IDENT_B          0x0B
#define QMC5883_REG_IDENT_C          0x20
#define QMC5883_REG_IDENT_D          0x21

// measurement bias
#define HMC5883L_CRA_BIAS_BIT       1
#define HMC5883L_CRA_BIAS_LENGTH    2

#define HMC5883L_BIAS_NORMAL        0x00
#define HMC5883L_BIAS_POSITIVE      0x01
#define HMC5883L_BIAS_NEGATIVE      0x02

// range
#define HMC5883L_CRB_RANGE_BIT       7
#define HMC5883L_CRB_RANGE_LENGTH    3

#define HMC5883L_RANGE_0_88G          0x00
#define HMC5883L_RANGE_1_3G           0x01
#define HMC5883L_RANGE_1_9G           0x02
#define HMC5883L_RANGE_2_5G           0x03
#define HMC5883L_RANGE_4_G            0x04
#define HMC5883L_RANGE_4_7G           0x05
#define HMC5883L_RANGE_5_6G           0x06
#define HMC5883L_RANGE_8_1G           0x07

#define QMC5883_CRB_RANGE_BIT        5
#define QMC5883_CRB_RANGE_LENGTH     2

#define QMC5883_RANGE_2G             0x00,
#define QMC5883_RANGE_8G             0x01

// sample averaging
#define HMC5883L_CRA_AVERAGE_BIT    6
#define HMC5883L_CRA_AVERAGE_LENGTH 2

#define HMC5883L_SAMPLES_8    0b11
#define HMC5883L_SAMPLES_4    0b10
#define HMC5883L_SAMPLES_2    0b01
#define HMC5883L_SAMPLES_1    0b00

#define QMC5883_CRA_AVERAGE_BIT     7
#define QMC5883_CRA_AVERAGE_LENGTH  2

#define QMC5883_SAMPLES_8     0b11
#define QMC5883_SAMPLES_4     0b10
#define QMC5883_SAMPLES_2     0b01
#define QMC5883_SAMPLES_1     0b00

// data rate
#define HMC5883L_CRA_RATE_BIT       4
#define HMC5883L_CRA_RATE_LENGTH    3

#define HMC5883L_DATARATE_75HZ       0b110
#define HMC5883L_DATARATE_30HZ       0b101
#define HMC5883L_DATARATE_15HZ       0b100
#define HMC5883L_DATARATE_7_5HZ      0b011
#define HMC5883L_DATARATE_3HZ        0b010
#define HMC5883L_DATARATE_1_5HZ      0b001
#define HMC5883L_DATARATE_0_75_HZ    0b000

#define QMC5883_CRA_RATE_BIT        3
#define QMC5883_CRA_RATE_LENGTH     2

#define QMC5883_DATARATE_10HZ        0b00
#define QMC5883_DATARATE_50HZ        0b01
#define QMC5883_DATARATE_100HZ       0b10
#define QMC5883_DATARATE_200HZ       0b11

// mode
#define HMC5883L_MODEREG_BIT        1
#define HMC5883L_MODEREG_LENGTH     2

#define HMC5883L_MODE_IDLE         0b10
#define HMC5883L_MODE_SINGLE       0b01
#define HMC5883L_MODE_CONTINOUS    0b00

#define QMC5883_MODEREG_BIT        1
#define QMC5883_MODEREG_LENGTH     2

#define QMC5883_MODE_STANDBY       0b00
#define QMC5883_MODE_CONTINUOUS    0b01

class Mag5883
{
public:
  Mag5883():isHMC_(false),isQMC_(false)
    {}
  bool initialize(void);

  bool testConnection();

  uint8_t getMeasurementBias();
  void setMeasurementBias(uint8_t bias);

  uint8_t getRange();
  void setRange(uint8_t gain);

  uint8_t getSampleAveraging();
  void setSampleAveraging(uint8_t averaging);

  uint8_t getDataRate();
  void setDataRate(uint8_t rate);

  uint8_t getMode();
  void setMode(uint8_t newMode);

  void getHeading(int16_t *x, int16_t *y, int16_t *z);
  
  int getICType(void);
  bool isHMC(){return isHMC_;}
  bool isQMC(){return isQMC_;}
private:
  bool isHMC_;
  bool isQMC_;
  uint8_t mode;
  uint8_t buffer[6];
};

#endif
