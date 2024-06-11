/*!
 * @file DFRobot_BloodOxygen_S.h
 * @brief This is the header of the library for the sensor that can detect human oxygen saturation and heart rate.
 * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author      PengKaixing(kaixing.peng@dfrobot.com)
 * @version  V1.0.0
 * @date  2021-06-21
 * @url https://github.com/DFRobot/DFRobot_BloodOxygen_S
 */
#ifndef __DFRobot_BloodOxygen_S_H__
#define __DFRobot_BloodOxygen_S_H__
#include <mbed.h>

// Open this macro to see the program running in detail
// #define ENABLE_DBG

#ifdef ENABLE_DBG
#define DBG(...)                 \
  {                              \
    Serial.print("[");           \
    Serial.print(__FUNCTION__);  \
    Serial.print("(): ");        \
    Serial.print(__LINE__);      \
    Serial.print(" ] ");         \
    Serial.println(__VA_ARGS__); \
  } 
#else
#define DBG(...)
#endif

#define MAX_SIZE 40
#define DEVICE_ADDRESS 0x20
#define BROADCAST_ADDRESS 0x00
#define RTU_READ_REG_CMD 0x03
#define RTU_WRITE_REG_CMD 0x06
#define RTU_WRITE_MULTIPLE_REG_CMD 0x10
#define SERIAL_DATA_BUF_MAX_SIZE 20
#define DEV_ADDRESS 0x0020

class DFRobot_BloodOxygen_S
{
  public:
    /**
     * @struct sHeartbeatSPO2
     * @brief The struct for storing heart rate and oxygen saturation
     */
    typedef struct
    {
      int SPO2;
      int Heartbeat;
    } sHeartbeatSPO2;

    DFRobot_BloodOxygen_S(void){};
    ~DFRobot_BloodOxygen_S(void){};

    /**
     * @fn getHeartbeatSPO2
     * @brief Get heart rate and oxygen saturation and store them into the struct sHeartbeatSPO2
     */
    void getHeartbeatSPO2(void);

    /**
     * @fn getTemperature_C
     * @brief Get the sensor board temp
     * @return The current onboard temp (unit: ℃)
     */
    float getTemperature_C(void);

    /**
     * @fn sensorStartCollect
     * @brief Data collecting starts
     */
    void sensorStartCollect(void);

    /**
     * @fn sensorEndCollect
     * @brief Data collecting stops
     */
    void sensorEndCollect(void);
    sHeartbeatSPO2 _sHeartbeatSPO2;

  protected:
    /**
     * @fn writeReg
     * @brief Write data to the specified register of the sensor
     * @param reg_addr Register address to be written
     * @param data_buf Data to be written to register
     * @param len Length of data to be written
     */
    virtual void writeReg(uint16_t reg_addr, uint8_t *data_buf, uint8_t len) = 0;

    /**
     * @fn readReg
     * @brief Get the data with specified length from the specified sensor
     * @param reg_addr Register address to be read
     * @param data_buf The position storing the register data to be read
     * @param len Length of the data to be read
     */
    virtual int16_t readReg(uint16_t reg_addr, uint8_t *data_buf, uint8_t len) = 0;
};

class DFRobot_BloodOxygen_S_I2C : public DFRobot_BloodOxygen_S
{
  public:
    DFRobot_BloodOxygen_S_I2C(uint8_t addr = 0xAE);
    ~DFRobot_BloodOxygen_S_I2C(){};
  protected:
    void writeReg(uint16_t reg_addr, uint8_t *data_buf, uint8_t len);
    int16_t readReg(uint16_t reg_addr, uint8_t *data_buf, uint8_t len);
  private:
    I2C *_i2c;
    uint8_t _I2C_addr;
};
#endif
