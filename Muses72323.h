/*
  The MIT License (MIT)

  Copyright (c) 2016 Christoffer Hjalmarsson

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef INCLUDED_MUSES_72323
#define INCLUDED_MUSES_72323

#include <Arduino.h>
#include "SPI.h"

//  ERROR CODES
#define Muses72323_OK                       0x00
//#define Muses72323_PIN_ERROR                0x81
//#define Muses72323_SPI_ERROR                0x82
#define Muses72323_VALUE_ERROR              0x83
//#define Muses72323_PORT_ERROR               0x84
//#define Muses72323_REGISTER_ERROR           0xFF

#ifndef __SPI_CLASS__
  #if defined(ARDUINO_ARCH_RP2040)
  #define __SPI_CLASS__   SPIClassRP2040
  #else
  #define __SPI_CLASS__   SPIClass
  #endif
#endif

const uint32_t Muses72323_TYP_SPI_SPEED =  800000;
const uint32_t Muses72323_MAX_SPI_SPEED = 1000000;

class Muses72323
{
public:
  // contextual data types
  typedef uint8_t pin_t;
  typedef uint16_t data_t;
  typedef int volume_t;
  typedef uint16_t address_t;

  //  Custom SPI
  Muses72323(uint8_t sclk, uint8_t miso, uint8_t mosi, uint8_t select, uint8_t chip_address = 0x00);
  //  HARDWARE SPI
  Muses72323(uint8_t select, __SPI_CLASS__ *spi);
  Muses72323(uint8_t select, uint8_t chip_address = 0x00, __SPI_CLASS__ *spi = &SPI);
  
  // set the pins in their correct states
  bool begin();
  bool begin(uint8_t sclk, uint8_t miso, uint8_t mosi, uint8_t select);

  // set the volume using the following formula:
  // (-0.25 * volume) db
  // audio level goes from -111.75 to 0.0 dB
  // input goes from -447 to 0
  uint8_t setVolume(volume_t left, volume_t right);

  // this function sets the settings in the gain address
  uint8_t setGain(data_t intgain);

  void mute();
  // must be set to false if no external clock is connected
  void setExternalClock(bool enabled);

  // enable or disable zero crossing
  void setZeroCrossingOn(bool enabled);

  // if set to true left and right will not be treated independently
  // to set attenuation with linked channels just set the left channel
  void setLinkChannels(bool enabled);

private:
  void transfer(address_t reg, data_t data);

  //       access to low level registers (just make these two functions public).
  //       USE WITH CARE !!!
  bool     writeReg(uint8_t reg, uint8_t value);

  uint8_t  _chip_address = 0;
  uint8_t  _select  = 0;
  uint8_t  _miso = 0;
  uint8_t  _mosi  = 0;
  uint8_t  _sclk   = 0;
  uint8_t  _error   = Muses72323_OK;

  bool     _hwSPI   = false;

  //  1 MHz is maximum, 800K is a better clock divider.
  uint32_t _SPIspeed = Muses72323_TYP_SPI_SPEED;
  __SPI_CLASS__ * _mySPI;
  SPISettings   _spi_settings;
 
  address_t chip_address;
  data_t states;
  data_t gain;

  uint8_t  swSPI_transfer(uint8_t val);
};

#endif // INCLUDED_MUSES_72323
