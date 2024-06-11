/*!
 * @file DFRobot_BloodOxygen_S.cpp
 * @brief This is the .cpp file for a sensor that can detect human oxygen saturation and heart rate.
 * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author      PengKaixing(kaixing.peng@dfrobot.com)
 * @version  V1.0.0
 * @date  2021-06-21
 * @url https://github.com/DFRobot/DFRobot_BloodOxygen_S
 */
#include "DFRobot_BloodOxygen_S.h"

void DFRobot_BloodOxygen_S::getHeartbeatSPO2(void)
{
  uint8_t SPO2Valid;
  int8_t HeartbeatValid;
  uint8_t rbuf[8];
  readReg(0x0C,rbuf,8);
  static uint8_t heartbeat_count;
  uint32_t SPO2_all_val=0;
  uint32_t heartbeat_all_val=0;
  _sHeartbeatSPO2.SPO2 = rbuf[0];
  if(_sHeartbeatSPO2.SPO2 == 0)
  {
    _sHeartbeatSPO2.SPO2 = -1;
  }
  _sHeartbeatSPO2.Heartbeat = ((uint32_t)rbuf[2] << 24) | ((uint32_t)rbuf[3] << 16) | ((uint32_t)rbuf[4] << 8) | ((uint32_t)rbuf[5]);
  if (_sHeartbeatSPO2.Heartbeat == 0)
  {
    _sHeartbeatSPO2.Heartbeat = -1;
  }
}

float DFRobot_BloodOxygen_S::getTemperature_C(void)
{
  uint8_t temp_buf[2];
  readReg(0x14, temp_buf, 2);
  float Temperature = temp_buf[0] * 1.0 + temp_buf[1] / 100.0;
  return Temperature;
}

void DFRobot_BloodOxygen_S::sensorStartCollect(void)
{
  uint8_t wbuf[2]={0,1};
  writeReg(0x20,wbuf,2);
}

void DFRobot_BloodOxygen_S::sensorEndCollect(void)
{
  uint8_t wbuf[2] = {0, 2};
  writeReg(0x20, wbuf, 2);
}

//I2C underlying communication
DFRobot_BloodOxygen_S_I2C::DFRobot_BloodOxygen_S_I2C(uint8_t addr)
{
  _i2c = new I2C(D14, D15);
  _I2C_addr = addr;
}

void DFRobot_BloodOxygen_S_I2C::writeReg(uint16_t reg_addr, uint8_t *data_buf, uint8_t len)
{
  uint8_t buffer[20];
  buffer[0] = reg_addr;
  for (int i=1; i<=len; i++) buffer[i] = data_buf[i-1];
  _i2c->write(_I2C_addr, (char *)buffer, len+1);
}

int16_t DFRobot_BloodOxygen_S_I2C::readReg(uint16_t reg_addr, uint8_t *data_buf, uint8_t len)
{
  uint8_t buffer[20];
  buffer[0] = reg_addr;
  _i2c->write(_I2C_addr, (char *)buffer, 1);
  _i2c->read(_I2C_addr, (char *)data_buf, len);
  return len;
}
