//
// hedrot-firmware
//
//
//
// Copyright	© Alexis Baskind, 2015-2017
// General Public License v3


// Axes reference: standard (Tait–Bryan):
// see for example Freescale Semiconductor Application Note, Document Number: AN4248, "Implementing a Tilt-Compensated eCompass using Accelerometer and Magnetometer Sensors"
// if we look at the teensy board with the daugther board on the top right (USB connector below left)
// x points towards the front
// y points towards the right
// z points towards the bottom


#include "ADXL345.h"
#include "Mag5883.h"
#include "ITG3200.h"
#include "hedrot_comm_protocol.h"
#include <EEPROM.h>
#include "i2c_t3.h"

#define LED_ON 1 // flag to use the imbedded LED

#define GYROSCOPE_HALFSCALE_SENSITIVITY 2000
#define GYROSCOPE_BITDEPTH 16


// EEPROM ADRESSES

//size in bytes of a calibration vector (6 floats, mag or accel) to store/recall in EEPROM (normally this should be 12 = 3 * 4bytes)
#define CALDATA_1VECTOR_EEPROM_SIZE (sizeof(float)*3)

//addresses of stored calibration vectors in EEPROM
#define SAMPLERATE_EEPROM_ADDRESS               0

#define ACC_OFFSET_EEPROM_ADDRESS               (SAMPLERATE_EEPROM_ADDRESS + sizeof(uint16_t))
#define ACC_SCALING_EEPROM_ADDRESS              (ACC_OFFSET_EEPROM_ADDRESS + CALDATA_1VECTOR_EEPROM_SIZE)

#define MAG_OFFSET_EEPROM_ADDRESS               (ACC_SCALING_EEPROM_ADDRESS + CALDATA_1VECTOR_EEPROM_SIZE)
#define MAG_SCALING_EEPROM_ADDRESS              (MAG_OFFSET_EEPROM_ADDRESS + CALDATA_1VECTOR_EEPROM_SIZE)

#define GYRO_RATE_EEPROM_ADDRESS                (MAG_SCALING_EEPROM_ADDRESS + CALDATA_1VECTOR_EEPROM_SIZE)
#define GYRO_CLOCK_SOURCE_EEPROM_ADDRESS        (GYRO_RATE_EEPROM_ADDRESS + 1)
#define GYRO_LPF_BANDWIDTH_EEPROM_ADDRESS       (GYRO_CLOCK_SOURCE_EEPROM_ADDRESS + 1)

#define ACC_RANGE_EEPROM_ADDRESS                (GYRO_LPF_BANDWIDTH_EEPROM_ADDRESS + 1)
#define ACC_HARD_OFFSET_EEPROM_ADDRESS          (ACC_RANGE_EEPROM_ADDRESS + 1)
#define ACC_FULLRESOLUTION_BIT_EEPROM_ADDRESS   (ACC_HARD_OFFSET_EEPROM_ADDRESS + 3)
#define ACC_DATARATE_EEPROM_ADDRESS             (ACC_FULLRESOLUTION_BIT_EEPROM_ADDRESS + 1)

#define MAG_MEASUREMENT_BIAS_EEPROM_ADDRESS     (ACC_DATARATE_EEPROM_ADDRESS + 1)
#define MAG_SAMPLE_AVERAGING_EEPROM_ADDRESS     (MAG_MEASUREMENT_BIAS_EEPROM_ADDRESS + 1)
#define MAG_DATA_RATE_EEPROM_ADDRESS            (MAG_SAMPLE_AVERAGING_EEPROM_ADDRESS + 1)
#define MAG_GAIN_EEPROM_ADDRESS                 (MAG_DATA_RATE_EEPROM_ADDRESS + 1)
#define MAG_MEASUREMENT_MODE_EEPROM_ADDRESS     (MAG_GAIN_EEPROM_ADDRESS + 1)


// class default I2C address is 0x53
// specific I2C addresses may be passed as a parameter here
// ALT low = 0x53 (default for SparkFun 6DOF board)
// ALT high = 0x1D
Mag5883 mag;
ITG3200 gyro;
ADXL345 accel;


// Local variables
char read_sensors = 0;
char transmitFlag = 0;
int16_t mx, my, mz;
int16_t gx, gy, gz;
int16_t ax, ay, az;


uint16_t samplerate; //Hz

long timeOfLastPing;

IntervalTimer readSensorsTimer;
void readSensorsTimertick()
{
    read_sensors++;
}

void startTransmission() {
    read_sensors = 0;
    transmitFlag = 1;
#if LED_ON
    digitalWrite(LED_BUILTIN, HIGH);
#endif
}

void stopTransmission() {
    transmitFlag = 0;
#if LED_ON
    digitalWrite(LED_BUILTIN, LOW);
#endif
}

void setup() {
    //Serial.println("Step1");
    // join I2C bus (I2Cdev library doesn't do this automatically)
    //on the specs of the ITG3200, the maximum I2C speed is 400kHz, but it seems to work up to 1500kHz
    Wire.begin(I2C_MASTER, 0, I2C_PINS_18_19, I2C_PULLUP_EXT, I2C_RATE_1500);
    //Serial.println("Step2");
    // initialize serial communication
    // (38400 chosen because it works as well at 8MHz as it does at 16MHz, but
    // it's really up to you depending on your project)
    Serial.begin(BAUDRATE);
    //Serial.println("Step3");
    
#if LED_ON
    pinMode(LED_BUILTIN, OUTPUT);
#endif
    //Serial.println("Step4");
    // initialize device
    //Serial.println("Initializing I2C devices...");
    accel.initialize();
    //Serial.println("Step5");
    accel.setIntDataReadyEnabled(1);
    //Serial.println("Step6");
    
    mag.initialize();
    //Serial.println("Step7");
    // configure gyroscope
    gyro.initialize();
    //Serial.println("Step8");
    gyro.setIntDataReadyEnabled(1);
    //Serial.println("Step9");
    if( !mag.testConnection() || !accel.testConnection() || !gyro.testConnection()) {
      // error connecting to the sensors
      // infinite loop with quick blinking
      /*if( !mag.testConnection()) {
        Serial.println("error connecting to Mag");
      } else {
        Serial.println("Connection to Mag OK");
      }
      
      if( !accel.testConnection()) {
        Serial.println("error connecting to Accel");
      } else {
        Serial.println("Connection to Accel OK");
      }

      if( !gyro.testConnection()) {
        Serial.println("error connecting to Gyro");
      } else {
        Serial.println("Connection to Gyro OK");
      }*/
      
      while(1) {
          digitalWrite(LED_BUILTIN, HIGH);
          delay(100);
          digitalWrite(LED_BUILTIN, LOW);
          delay(100);
      }
    } else {
      // no error while connecting
      // blink twice slowly and go further
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000);
        digitalWrite(LED_BUILTIN, LOW);
        delay(1000);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000);
        digitalWrite(LED_BUILTIN, LOW);
    }

    stopTransmission();
    
    samplerate = 1000;
    
    readSensorsTimer.begin(readSensorsTimertick, 1000000/samplerate);
    interrupts();
}

inline void SendData(int16_t vx, int16_t vy, int16_t vz) {
    // send the data in 7 bits, the MSB being always 1
    // for each sensor with values vx, vy, vz in 16 bits:
    // 1/vx_1-vx_7
    Serial.write(128 | ((unsigned short)vx >> 9));
    // 1/vx_8-vx_14
    Serial.write(128 | ((unsigned short)(vx<<7) >> 9));
    // 1/vx_15/vx_16/vy_1-vy_5
    Serial.write(128 | (((unsigned short)(vx<<14)) >> 9) | ((unsigned short)vy >> 11));
    // 1/vy_6-vx_12
    Serial.write(128 | (((unsigned short)(vy<<5)) >> 9));
    // 1/vy_13-vy_16/vz_1-vz_3
    Serial.write(128 | (((unsigned short)(vy<<12)) >> 9) | ((unsigned short)vz >> 13));
    // 1/vz_4-vy_10
    Serial.write(128 | (((unsigned short)(vz<<3)) >> 9));
    // 1/vz_11-vy_16/0
    Serial.write(128 | (((unsigned short)(vz<<10)) >> 9) | 1);
}



void storeCalDataInEEPROM(float* calData, unsigned char EEPROM_address, unsigned char numberOfBytes) {
    byte *ptr = (byte *)calData;
    for (byte i = 0; i < numberOfBytes; i++)
        EEPROM.write(EEPROM_address + i, *ptr++);
}




void recallCalDataFromEEPROM(float* calData, unsigned char EEPROM_address, unsigned char numberOfBytes) {
    byte *ptr = (byte *)calData;
    for (byte i = 0; i < numberOfBytes; i++)
        *ptr++ = EEPROM.read(EEPROM_address + i);
}




int receive3calibrationValues(float* calData, unsigned char stop_transmit_char) {
    int i = 0;
    int signValue = 0;
    int TMPval = 0;
    int numberOfDigitsAfterDot = -1;
    
    unsigned char readChar;
    
    int continueReading = 1;
    while(continueReading) {
        readChar = Serial.read();
        if(readChar==DOT) { // dot, start to count the number of digits after the dot if it makes sense
            if(numberOfDigitsAfterDot==-1) {
                numberOfDigitsAfterDot = 0;
            } else { //already defined, error
                return 0;
            }
        } else if(readChar==SPACE) { //space, the transmission of the previous number is finished, go to the next one
            // first check if the sign value has be previously defined (i.e. if there was at least the minus character or a digit since the last space)
            if(signValue==0) { // no, returns error
                return 0;
            } else { // looks ok, the number can be stored
                // stores the number
                if(numberOfDigitsAfterDot==-1) { // the number is not decimal but integer
                    numberOfDigitsAfterDot=0;
                }
                calData[i]=signValue*TMPval/pow(10,numberOfDigitsAfterDot);
                
                //Serial.print("i: ");
                //Serial.print(i);
                //Serial.print(",calData[i]: ");
                //Serial.println(calData[i]);
                // resets the variables and increment the index
                signValue = 0;
                TMPval = 0;
                numberOfDigitsAfterDot = -1;
                i++;
                if(i==3) { //stops reading
                    continueReading = 0;
                }
            }
        } else if(readChar==MINUS) { //minus, can be considered as a negative number only if the sign has still not been defined
            if(signValue==0) { // new number, sign not defined
                signValue=-1;
            } else { // error
                return 0;
            }
        } else if((readChar>=ASCII_0) && (readChar<=ASCII_9)) { // if it's a digit
            
            // check if the sign has been defined. If not, the sign is positive
            if(signValue==0) { // new number, sign not defined
                signValue=1;
            }
            TMPval=TMPval*10+readChar-ASCII_0;
            if(numberOfDigitsAfterDot>=0) {
                numberOfDigitsAfterDot++;
            }
        } else if(readChar==stop_transmit_char) { //if the stream is finished
            // check if it should be finished (if we read all the 3 values)
            if(i!=2) { // no, returns error
                return 0;
            } else {
                // store the last value
                if(numberOfDigitsAfterDot==-1) { // the number is not decimal but integer
                    numberOfDigitsAfterDot=0;
                }
                calData[i]=signValue*TMPval/pow(10,numberOfDigitsAfterDot);
                continueReading = 0;
            }
        }
    }
    
    return 1;
}

void send3calibrationValues(float* calData) {
    // send 3 float values in ASCII, separated by spaces
    Serial.print(calData[0]);
    Serial.print(" ");
    Serial.print(calData[1]);
    Serial.print(" ");
    Serial.print(calData[2]);
}


void transmitInfo() {
    // ------------------------------- 1: GLOBAL INFOS ---------------------------------------------
    Serial.write(H2R_START_TRANSMIT_INFO_CHAR);//means "start transmitting info"
    
    // Sensor Board Type
    // 0 = gy-85 with Honeywell HMC5883L Magnetometer
    // 1 = gy-85 with QMC5883L Magnetometer
    Serial.print("sensor_board_type ");Serial.print( mag.isHMC() ? 0 : 1 );
    Serial.print(",");
    
    Serial.print("firmware_version ");Serial.print(HEDROT_FIRMWARE_VERSION);
    Serial.print(",");
    
    //read samplerate from EEPROM (unsigned int = 2 bytes)
    byte *ptr = (byte *) &samplerate;
    for (byte i = 0; i < 2; i++)
        *ptr++ = EEPROM.read(SAMPLERATE_EEPROM_ADDRESS + i);
    if(samplerate==0)
        samplerate=1000;
    //set sample rate
    readSensorsTimer.end();
    readSensorsTimer.begin(readSensorsTimertick, 1000000/samplerate);
    Serial.print("samplerate ");Serial.print(samplerate);
    Serial.print(",");
    
    
    
    
    // ------------------------------- 2: GYROSCOPE INFOS ---------------------------------------------
    Serial.print("gyroHalfScaleSensitivity ");Serial.print(GYROSCOPE_HALFSCALE_SENSITIVITY);
    Serial.print(",");
    Serial.print("gyroBitDepth ");Serial.print(GYROSCOPE_BITDEPTH);
    Serial.print(",");
    
    //read gyroscope data rate from EEPROM
    char gyroDataRate = EEPROM.read(GYRO_RATE_EEPROM_ADDRESS);
    //set accelerometer range and read it for double-checking
    gyro.setRate(gyroDataRate);
    Serial.print("gyroDataRate ");Serial.print(gyro.getRate());
    Serial.print(",");
    
    //read gyroscope clock source from EEPROM
    char gyroClockSource = EEPROM.read(GYRO_CLOCK_SOURCE_EEPROM_ADDRESS);
    //set accelerometer range and read it for double-checking
    gyro.setClockSource(gyroClockSource);
    Serial.print("gyroClockSource ");Serial.print(gyro.getClockSource());
    Serial.print(",");
    
    //read gyroscope low-pass filter bandwidth from EEPROM
    char gyroDLPFBandwidth = EEPROM.read(GYRO_LPF_BANDWIDTH_EEPROM_ADDRESS);
    //set accelerometer range and read it for double-checking
    gyro.setDLPFBandwidth(gyroDLPFBandwidth);
    Serial.print("gyroDLPFBandwidth ");Serial.print(gyro.getDLPFBandwidth());
    Serial.print(",");
    
    Serial.print("gyroscope_data_ready_enabled ");Serial.print(gyro.getIntDataReadyEnabled());
    Serial.print(",");
    
    
    
    // ------------------------------- 3: ACCELEROMETER INFOS ---------------------------------------------
    int8_t accOffsetX, accOffsetY, accOffsetZ;
    //read accelerometer hard offset data from EEPROM
    char accHardOffsetBuffer[3];
    for (byte i = 0; i < 3; i++)
        accHardOffsetBuffer[i] = EEPROM.read(ACC_HARD_OFFSET_EEPROM_ADDRESS + i);
    //set accelerometer offset and read it for double-checking
    accel.setOffset(accHardOffsetBuffer[1], accHardOffsetBuffer[0], accHardOffsetBuffer[2]); // y and x axes are inversed on the accelerometer
    accel.getOffset(&accOffsetY, &accOffsetX, &accOffsetZ); //x and y axes are inversed on the accelerometer
    Serial.print("accHardOffset ");Serial.print(accOffsetX);Serial.print(" ");Serial.print(accOffsetY);Serial.print(" ");Serial.print(accOffsetZ);
    Serial.print(",");
    
    //read accelerometer full resolution bit from EEPROM
    char accFullResolutionBit = EEPROM.read(ACC_FULLRESOLUTION_BIT_EEPROM_ADDRESS);
    //set accelerometer full resolution bit and read it for double-checking
    accel.setFullResolution(accFullResolutionBit);
    Serial.print("accFullResolutionBit ");Serial.print((accel.getFullResolution()!=0));
    Serial.print(",");
    
    //read accelerometer data rate from EEPROM
    char accDataRate = EEPROM.read(ACC_DATARATE_EEPROM_ADDRESS);
    //set accelerometer data rate and read it for double-checking
    accel.setRate(accDataRate);
    Serial.print("accDataRate ");Serial.print(accel.getRate());
    Serial.print(",");
    
    //read accelerometer range from EEPROM
    char accRange = EEPROM.read(ACC_RANGE_EEPROM_ADDRESS);
    //set accelerometer range and read it for double-checking
    accel.setRange(accRange);
    Serial.print("accRange ");Serial.print(accel.getRange());
    Serial.print(",");
    
    /*Serial.print("accelerometer_lowPowerStatus ");Serial.print(accel.getLowPowerEnabled());
    Serial.print(",");
    Serial.print("accelerometer_selfTestEnabledBit ");Serial.print(accel.getSelfTestEnabled());
    Serial.print(",");*/
    
    // read accelerometer cal data from EEPROM
    float accCalOffsetData[3];
    recallCalDataFromEEPROM(accCalOffsetData, ACC_OFFSET_EEPROM_ADDRESS, CALDATA_1VECTOR_EEPROM_SIZE);
    Serial.print("accOffset ");send3calibrationValues(accCalOffsetData);
    Serial.print(",");
    
    float accCalScalingData[3];
    recallCalDataFromEEPROM(accCalScalingData, ACC_SCALING_EEPROM_ADDRESS, CALDATA_1VECTOR_EEPROM_SIZE);
    Serial.print("accScaling ");send3calibrationValues(accCalScalingData);
    Serial.print(",");
    
    
    // ------------------------------- 4: MAGNETOMETER INFOS ---------------------------------------------
    //read magnetometer measurement bias from EEPROM
    char magMeasurementBias = EEPROM.read(MAG_MEASUREMENT_BIAS_EEPROM_ADDRESS);
    //set magnetometer measurement bias and read it for double-checking
    mag.setMeasurementBias(magMeasurementBias);
    Serial.print("magMeasurementBias ");Serial.print(mag.getMeasurementBias());
    Serial.print(",");
    
    //read magnetometer sample averaging from EEPROM
    char magSampleAveraging = EEPROM.read(MAG_SAMPLE_AVERAGING_EEPROM_ADDRESS);
    //set magnetometer magnetometer sample averaging and read it for double-checking
    mag.setSampleAveraging(magSampleAveraging);
    Serial.print("magSampleAveraging ");Serial.print(mag.getSampleAveraging());
    Serial.print(",");
    
    
    //read magnetometer data rate from EEPROM
    char magDataRate = EEPROM.read(MAG_DATA_RATE_EEPROM_ADDRESS);
    //set magnetometer data rate and read it for double-checking
    mag.setDataRate(magDataRate);
    Serial.print("magDataRate ");Serial.print(mag.getDataRate());
    Serial.print(",");
    
    //read magnetometer gain from EEPROM
    char magGain = EEPROM.read(MAG_GAIN_EEPROM_ADDRESS);
    //set magnetometer gain and read it for double-checking
    mag.setRange(magGain);
    Serial.print("magGain ");Serial.print(mag.getRange());
    Serial.print(",");
    
    //read magnetometer measurement mode from EEPROM
    char magMeasurementMode = EEPROM.read(MAG_MEASUREMENT_MODE_EEPROM_ADDRESS);
    //set magnetometer measurement mode and read it for double-checking
    mag.setMode(magMeasurementMode);
    Serial.print("magMeasurementMode ");Serial.print(mag.getMode() & 1); //only the lsb
    Serial.print(",");
    
    // read magnetometer cal data from EEPROM
    float magCalOffsetData[3];
    recallCalDataFromEEPROM(magCalOffsetData, MAG_OFFSET_EEPROM_ADDRESS, CALDATA_1VECTOR_EEPROM_SIZE);
    Serial.print("magOffset ");send3calibrationValues(magCalOffsetData);
    Serial.print(",");
    
    float magCalScalingData[3];
    recallCalDataFromEEPROM(magCalScalingData, MAG_SCALING_EEPROM_ADDRESS, CALDATA_1VECTOR_EEPROM_SIZE);
    Serial.print("magScaling ");send3calibrationValues(magCalScalingData);
    
    Serial.write(H2R_STOP_TRANSMIT_INFO_CHAR);//means "info transmitted"
    
}

void loop() {
    
    // if the last ping occured more than 1sec ago, stop the transmission
    if(millis()-timeOfLastPing>=1000)
        stopTransmission();
    
    if(Serial.available()) { // if new data is incoming
        switch(Serial.read()) {
            case R2H_STOP_TRANSMISSION_CHAR: //stop transmission
                stopTransmission();
                break;
            case R2H_SEND_INFO_CHAR: //stop transmission & send info
                stopTransmission();
                transmitInfo();
                break;
            case R2H_TRANSMIT_SAMPLERATE: // receiving samplerate
            {
                if(Serial.readBytes((char *) &samplerate,2)==0) { //error while reading
                    Serial.write(H2R_DATA_RECEIVE_ERROR_CHAR);
                } else {
                    if(samplerate==0)
                        samplerate=1000;
                    //set sample rate
                    readSensorsTimer.end();
                    readSensorsTimer.begin(readSensorsTimertick, 1000000/samplerate);
                    
                    //write samplerate from EEPROM (unsigned int = 2 bytes)
                    byte *ptr = (byte *) &samplerate;
                    for (byte i = 0; i < 2; i++)
                        EEPROM.write(SAMPLERATE_EEPROM_ADDRESS + i,*ptr++);
                }
            }
                break;
            case R2H_TRANSMIT_GYRO_RATE: // receiving gyro rate
            {
                char gyroDataRate = Serial.read();
                //set gyro rate
                gyro.setRate(gyroDataRate);
                // write in EEPROM
                EEPROM.write(GYRO_RATE_EEPROM_ADDRESS, gyroDataRate);
            }
                break;
            case R2H_TRANSMIT_GYRO_CLOCK_SOURCE: // receiving gyro clock source
            {
                char gyroClockSource = Serial.read();
                //set gyro clock source
                gyro.setClockSource(gyroClockSource);
                // write in EEPROM
                EEPROM.write(GYRO_CLOCK_SOURCE_EEPROM_ADDRESS, gyroClockSource);
            }
                break;
            case R2H_TRANSMIT_GYRO_LPF_BANDWIDTH: // receiving gyro low-pass filter bandwidth
            {
                char gyroDLPFBandwidth = Serial.read();
                //set gyro low-pass filter bandwidth
                gyro.setDLPFBandwidth(gyroDLPFBandwidth);
                // write in EEPROM
                EEPROM.write(GYRO_LPF_BANDWIDTH_EEPROM_ADDRESS, gyroDLPFBandwidth);
            }
                break;
            case R2H_TRANSMIT_ACCEL_RANGE: // receiving accel range
            {
                char accRange = Serial.read();
                //set accelerometer range
                accel.setRange(accRange);
                // write in EEPROM
                EEPROM.write(ACC_RANGE_EEPROM_ADDRESS, accRange);
            }
                break;
            case R2H_START_TRANSMIT_ACCEL_HARD_OFFSET: // start receiving accel offset (3 bytes, x, y, z)
            {
                char buffer[4];
                if(Serial.readBytes(buffer,4)==0) { //error while reading
                    Serial.write(H2R_DATA_RECEIVE_ERROR_CHAR);
                } else {
                    if(buffer[3] == R2H_STOP_TRANSMIT_ACCEL_HARD_OFFSET) { // transmission ok
                        accel.setOffset(buffer[1], buffer[0], buffer[2]); // y and x axes are inversed on the accelerometer
                        //store in EEPROM
                        for (byte i = 0; i < 3; i++)
                            EEPROM.write(ACC_HARD_OFFSET_EEPROM_ADDRESS + i, buffer[i]);
                    } else {
                        Serial.write(H2R_DATA_RECEIVE_ERROR_CHAR);
                    }
                }
            }
                break;
            case R2H_TRANSMIT_ACCEL_FULL_RESOLUTION_BIT: // receiving accel full resolution bit
            {
                char accFullResolutionBit = Serial.read();
                //set accelerometer range
                accel.setFullResolution(accFullResolutionBit);
                // write in EEPROM
                EEPROM.write(ACC_FULLRESOLUTION_BIT_EEPROM_ADDRESS, accFullResolutionBit);
            }
                break;
            case R2H_TRANSMIT_ACCEL_DATARATE: // receiving accel data rate
            {
                char accDataRate = Serial.read();
                //set accelerometer range
                accel.setRate(accDataRate);
                // write in EEPROM
                EEPROM.write(ACC_DATARATE_EEPROM_ADDRESS, accDataRate);
            }
                break;
            case R2H_TRANSMIT_MAG_MEASUREMENT_BIAS: // receiving magnetometer measurement bias
            {
                char magMeasurementBias = Serial.read();
                //set magnetometer measurement bias
                mag.setMeasurementBias(magMeasurementBias);
                // write in EEPROM
                EEPROM.write(MAG_MEASUREMENT_BIAS_EEPROM_ADDRESS, magMeasurementBias);
            }
                break;
            case R2H_TRANSMIT_MAG_SAMPLE_AVERAGING: // receiving magnetometer sample averaging
            {
                char magSampleAveraging = Serial.read();
                //set magnetometer sample averaging
                mag.setSampleAveraging(magSampleAveraging);
                // write in EEPROM
                EEPROM.write(MAG_SAMPLE_AVERAGING_EEPROM_ADDRESS, magSampleAveraging);
            }
                break;
            case R2H_TRANSMIT_MAG_DATA_RATE: // receiving magnetometer data rate
            {
                char magDataRate = Serial.read();
                //set magnetometer data rate
                mag.setDataRate(magDataRate);
                // write in EEPROM
                EEPROM.write(MAG_DATA_RATE_EEPROM_ADDRESS, magDataRate);
            }
                break;
            case R2H_TRANSMIT_MAG_GAIN: // receiving magnetometer gain
            {
                char magGain = Serial.read();
                //set magnetometer gain
                mag.setRange(magGain);
                // write in EEPROM
                EEPROM.write(MAG_GAIN_EEPROM_ADDRESS, magGain);
            }
                break;
            case R2H_TRANSMIT_MAG_MEASUREMENT_MODE: // receiving magnetometer measurement mode
            {
                char magMeasurementMode = Serial.read();
                //set magnetometer measurement mode
                mag.setMode(magMeasurementMode);
                // write in EEPROM
                EEPROM.write(MAG_MEASUREMENT_MODE_EEPROM_ADDRESS, magMeasurementMode);
            }
                break;
            case R2H_START_TRANSMIT_ACCEL_OFFSET_DATA_CHAR: //start receiving acc offset calibration info
            {
                // needs to receive 3 values having the form XXXXX.YY. If something went wrong, does not save and send an error
                float accCalOffsetData[6];
                int err = receive3calibrationValues(accCalOffsetData,R2H_STOP_TRANSMIT_ACCEL_OFFSET_DATA_CHAR);
                if(err == 0) { //error while receiving the values
                    Serial.write(H2R_DATA_RECEIVE_ERROR_CHAR);
                } else { //reception ok, store the values
                    storeCalDataInEEPROM(accCalOffsetData,ACC_OFFSET_EEPROM_ADDRESS,CALDATA_1VECTOR_EEPROM_SIZE);
                }
            }
                break;
            case R2H_START_TRANSMIT_ACCEL_SCALING_DATA_CHAR: //start receiving acc scaling calibration info
            {
                // needs to receive 3 values having the form XXXXX.YY. If something went wrong, does not save and send an error
                float accCalScalingData[6];
                int err = receive3calibrationValues(accCalScalingData,R2H_STOP_TRANSMIT_ACCEL_SCALING_DATA_CHAR);
                if(err == 0) { //error while receiving the values
                    Serial.write(H2R_DATA_RECEIVE_ERROR_CHAR);
                } else { //reception ok, store the values
                    storeCalDataInEEPROM(accCalScalingData,ACC_SCALING_EEPROM_ADDRESS,CALDATA_1VECTOR_EEPROM_SIZE);
                }
            }
                break;
            case R2H_START_TRANSMIT_MAG_OFFSET_DATA_CHAR: //start receiving mag offset calibration info
            {
                // needs to receive 3 values having the form XXXXX.YY. If something went wrong, does not save and send an error
                float magCalOffsetData[6];
                int err = receive3calibrationValues(magCalOffsetData,R2H_STOP_TRANSMIT_MAG_OFFSET_DATA_CHAR);
                if(err == 0) { //error while receiving the values
                    Serial.write(H2R_DATA_RECEIVE_ERROR_CHAR);
                } else { //reception ok, store the values
                    storeCalDataInEEPROM(magCalOffsetData,MAG_OFFSET_EEPROM_ADDRESS,CALDATA_1VECTOR_EEPROM_SIZE);
                }
            }
                break;
            case R2H_START_TRANSMIT_MAG_SCALING_DATA_CHAR: //start receiving mag scaling calibration info
            {
                // needs to receive 3 values having the form XXXXX.YY. If something went wrong, does not save and send an error
                float magCalScalingData[6];
                int err = receive3calibrationValues(magCalScalingData,R2H_STOP_TRANSMIT_MAG_SCALING_DATA_CHAR);
                if(err == 0) { //error while receiving the values
                    Serial.write(H2R_DATA_RECEIVE_ERROR_CHAR);
                } else { //reception ok, store the values
                    storeCalDataInEEPROM(magCalScalingData,MAG_SCALING_EEPROM_ADDRESS,CALDATA_1VECTOR_EEPROM_SIZE);
                }
            }
                break;
            case R2H_AREYOUTHERE_CHAR: //special ping during autodiscovering
                Serial.write(H2R_IAMTHERE_CHAR);
                break;
            case R2H_PING_CHAR: //ping
                // if the transmission did not start already, start it
                if(!transmitFlag)
                    startTransmission();
                
                Serial.write(H2R_PING_CHAR);
#if LED_ON
                if(!transmitFlag) {
                    digitalWrite(LED_BUILTIN, HIGH);
                    delay(10);
                    digitalWrite(LED_BUILTIN, LOW);
                    delay(10);
                }
#endif
                timeOfLastPing = millis(); // updates the variable containing the time of the last ping
                break;
        }
    }
    
    // if "read_sensors" flag is set high, read sensors and update
    if (read_sensors && transmitFlag) {
#if 0  // return to zero
        if(readByte(ADXL345_DEFAULT_ADDRESS, ADXL345_RA_INT_SOURCE) & 0b10000000) //équivalent à accel.getIntDataReadySource()
            accel.getAcceleration(&ax, &ay, &az);
        else {
            ax = 0;
            ay = 0;
            az = 0;
        }
        
        if(readByte(HMC5883L_DEFAULT_ADDRESS, HMC5883L_RA_STATUS) & 1) //équivalent à mag.getReadyStatus()
            mag.getHeading(&mx, &my, &mz);
        else {
            mx = 0;
            my = 0;
            mz = 0;
        }
        
        if(readByte(ITG3200_DEFAULT_ADDRESS, ITG3200_RA_INT_STATUS) & 1) //équivalent à gyro.getIntDataReadyStatus()
            gyro.getRotation(&gx, &gy, &gz);
        else {
            gx = 0;
            gy = 0;
            gz = 0;
        }
#else //sample and hold
        accel.getAcceleration(&ax, &ay, &az);
        mag.getHeading(&mx, &my, &mz);
        gyro.getRotation(&gx, &gy, &gz);
#endif
        
        // send magnetometer data:
        SendData(-my,-mx,-mz);
        
        // send accelerometer data:
        SendData(ay,ax,az);
        
        // send gyroscope data:
        SendData(-gy,-gx,-gz);
        
        Serial.write(H2R_END_OF_RAWDATA_FRAME); // closes the frame
        
        Serial.send_now();
        
        if(read_sensors>1) { 
            // error: acquiring, preparing and sending the data is too slow on this board
            Serial.write(H2R_BOARD_OVERLOAD); // "too slow"
        }
        
        
        // Will first update read_sensors when everything is done.  
        read_sensors = 0;
        
    }
}


