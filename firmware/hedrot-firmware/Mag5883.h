/*!
 * @file Mag5883.h
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

#ifndef Mag5883_H
#define Mag5883_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define HMC5883L_ADDRESS             (0x1E)
#define QMC5883_ADDRESS              (0x0D)



#define HMC5883L_REG_CONFIG_A         (0x00)
#define HMC5883L_REG_CONFIG_B         (0x01)
#define HMC5883L_REG_MODE             (0x02)
#define HMC5883L_REG_OUT_X_M          (0x03)
#define HMC5883L_REG_OUT_X_L          (0x04)
#define HMC5883L_REG_OUT_Z_M          (0x05)
#define HMC5883L_REG_OUT_Z_L          (0x06)
#define HMC5883L_REG_OUT_Y_M          (0x07)
#define HMC5883L_REG_OUT_Y_L          (0x08)
#define HMC5883L_REG_STATUS           (0x09)
#define HMC5883L_REG_IDENT_A          (0x0A)
#define HMC5883L_REG_IDENT_B          (0x0B)
#define HMC5883L_REG_IDENT_C          (0x0C)


#define QMC5883_REG_OUT_X_M          (0x01)
#define QMC5883_REG_OUT_X_L          (0x00)
#define QMC5883_REG_OUT_Z_M          (0x05)
#define QMC5883_REG_OUT_Z_L          (0x04)
#define QMC5883_REG_OUT_Y_M          (0x03)
#define QMC5883_REG_OUT_Y_L          (0x02)
#define QMC5883_REG_STATUS           (0x06)
#define QMC5883_REG_CONFIG_1         (0x09)
#define QMC5883_REG_CONFIG_2         (0x0A)
#define QMC5883_REG_IDENT_B          (0x0B)
#define QMC5883_REG_IDENT_C          (0x20)
#define QMC5883_REG_IDENT_D          (0x21)

// measurement bias
#define HMC5883L_CRA_BIAS_BIT       1
#define HMC5883L_CRA_BIAS_LENGTH    2

// gain
#define HMC5883L_CRB_GAIN_BIT       7
#define HMC5883L_CRB_GAIN_LENGTH    3

// sample averaging
#define HMC5883L_CRA_AVERAGE_BIT    6
#define HMC5883L_CRA_AVERAGE_LENGTH 2

// data rate
#define HMC5883L_CRA_RATE_BIT       4
#define HMC5883L_CRA_RATE_LENGTH    3

// mode
#define HMC5883L_MODEREG_BIT        1
#define HMC5883L_MODEREG_LENGTH     2

#define HMC5883L_MODE_CONTINUOUS    0x00
#define HMC5883L_MODE_SINGLE        0x01
#define HMC5883L_MODE_IDLE          0x02



typedef enum
{
  HMC5883L_SAMPLES_8    = 0b11,
  HMC5883L_SAMPLES_4    = 0b10,
  HMC5883L_SAMPLES_2    = 0b01,
  HMC5883L_SAMPLES_1    = 0b00,

  QMC5883_SAMPLES_8     = 0b11,
  QMC5883_SAMPLES_4     = 0b10,
  QMC5883_SAMPLES_2     = 0b01,
  QMC5883_SAMPLES_1     = 0b00
} QMC5883_samples_t;

typedef enum
{
  HMC5883L_DATARATE_75HZ       = 0b110,
  HMC5883L_DATARATE_30HZ       = 0b101,
  HMC5883L_DATARATE_15HZ       = 0b100,
  HMC5883L_DATARATE_7_5HZ      = 0b011,
  HMC5883L_DATARATE_3HZ        = 0b010,
  HMC5883L_DATARATE_1_5HZ      = 0b001,
  HMC5883L_DATARATE_0_75_HZ    = 0b000,

  QMC5883_DATARATE_10HZ        = 0b00,
  QMC5883_DATARATE_50HZ        = 0b01,
  QMC5883_DATARATE_100HZ       = 0b10,
  QMC5883_DATARATE_200HZ       = 0b11
} QMC5883_dataRate_t;

typedef enum
{
  HMC5883L_RANGE_8_1GA     = 0b111,
  HMC5883L_RANGE_5_6GA     = 0b110,
  HMC5883L_RANGE_4_7GA     = 0b101,
  HMC5883L_RANGE_4GA       = 0b100,
  HMC5883L_RANGE_2_5GA     = 0b011,
  HMC5883L_RANGE_1_9GA     = 0b010,
  HMC5883L_RANGE_1_3GA    = 0b001,
  HMC5883L_RANGE_0_88GA    = 0b000,

  QMC5883_RANGE_2GA     = 0b00,
  QMC5883_RANGE_8GA     = 0b01
} QMC5883_range_t;

typedef enum
{
  HMC5883L_IDLE         = 0b10,
  HMC5883_SINGLE        = 0b01,
  HMC5883L_CONTINOUS    = 0b00,

  QMC5883_SINGLE        = 0b00,
  QMC5883_CONTINOUS     = 0b01
} QMC5883_mode_t;

#define IC_NONE     0
#define IC_HMC5883L 1
#define IC_QMC5883  2



#ifndef VECTOR_STRUCT_H
#define VECTOR_STRUCT_H
struct Vector
{
  float XAxis;
  float YAxis;
  float ZAxis;
};
#endif

class Mag5883
{
public:
  Mag5883():isHMC_(false),isQMC_(false),minX(0),maxX(0),minY(0), maxY(0), minZ(0), maxZ(0),firstRun(true)
    {}
  bool initialize(void);

  bool testConnection();

  uint8_t getMeasurementBias();
  void setMeasurementBias(uint8_t bias);

  uint8_t getGain();
  void setGain(uint8_t gain);

  uint8_t getSampleAveraging();
  void setSampleAveraging(uint8_t averaging);

  uint8_t getDataRate();
  void setDataRate(uint8_t rate);

  uint8_t getMode();
  void setMode(uint8_t newMode);

  void getHeading(int16_t *x, int16_t *y, int16_t *z);

  //Vector readRaw(void);
  
  //void initMinMax();
  //void calibrate(void);
  
  //void  setRange(QMC5883_range_t range);
  //QMC5883_range_t getRange(void);

  //void  setMeasurementMode(QMC5883_mode_t mode);
  //QMC5883_mode_t getMeasurementMode(void);

  //void  setDataRate(QMC5883_dataRate_t dataRate);
  //QMC5883_dataRate_t getDataRate(void);

  //void  setSamples(QMC5883_samples_t samples);
  //QMC5883_samples_t getSamples(void);
  
  int getICType(void);
  bool isHMC(){return isHMC_;}
  bool isQMC(){return isQMC_;}
private:
  bool isHMC_;
  bool isQMC_;
  uint8_t mode;
  uint8_t buffer[6];
  
  Vector v;
  
  
  float minX, maxX;
  float minY, maxY;
  float minZ, maxZ;
  bool firstRun;
  void writeRegister8(uint8_t reg, uint8_t value);
  uint8_t readRegister8(uint8_t reg);
  uint8_t fastRegister8(uint8_t reg);
  int16_t readRegister16(uint8_t reg);
};

#endif
