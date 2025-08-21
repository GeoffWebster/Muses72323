/*
  The MIT License (MIT)

  Copyright (c) 2016 Christoffer Hjalmarsson
  Copyright (c) 2025 Geoff Webster

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

#include "Muses72323.h"
#include <SPI.h>

typedef Muses72323 Self;

// pass through some types into global scope
using data_t = Self::data_t;
using volume_t = Self::volume_t;
using pin_t = Self::pin_t;

// control select addresses, chip address (low 2) ignored
static const data_t s_control_attenuation_l = 0b0000000000010000;
static const data_t s_control_attenuation_r = 0b0000000000010100;
static const data_t s_control_gain = 0b0000000000001000;
static const data_t s_control_states = 0b0000001000001100;

// control state bits
static const data_t s_state_soft_step = 4;
static const data_t s_state_bit_zero_crossing = 8;
static const data_t s_state_external_clock = 9;
static const data_t s_state_bit_gain = 15;

pin_t _select;
// Muses72323 max clock freq=1MHz, set for 800KHz
static const SPISettings s_muses_spi_settings(800000, MSBFIRST, SPI_MODE0);

static inline data_t volume_to_attenuation(volume_t volume)
{
  volume_t tmp;
  tmp = 4096 + (-volume * 128);
  // volume to attenuation data conversion:
  // #=============================================#
  // |    0.0 dB | in: [  0] -> 0b0001000000000000 |
  // | -117.5 dB | in: [447] -> 0b1110111110000000 |
  // #=============================================#
  return static_cast<data_t>(tmp);
}
/*
  static inline data_t volume_to_gain(volume_t gain)
  {
  // volume to gain data conversion:
  // #===================================#
  // |     0 dB | in: [ 0] -> 0b00000000 |
  // | +31.5 dB | in: [63] -> 0b01111111 |
  // #===================================#
  return static_cast<data_t>(min(gain, 63));
  }
*/

//  Custom SPI
Self::Muses72323(uint8_t sclk, uint8_t miso, uint8_t mosi, uint8_t select, uint8_t chip_address)
{
  _chip_address = (chip_address << 1);
  _select = select;
  _mosi = mosi;
  _miso = miso;
  _sclk = sclk;
  _error = Muses72323_OK;
  _hwSPI = false;
}

//  HARDWARE SPI
Self::Muses72323(uint8_t select, __SPI_CLASS__ *spi)
{
  Muses72323(select, 0x00, spi);
}

//  HARDWARE SPI
Self::Muses72323(uint8_t select, uint8_t chip_address, __SPI_CLASS__ *spi)
{
  _chip_address = (chip_address << 1);
  _select = select;
  _error = Muses72323_OK;
  _mySPI = spi;
  _hwSPI = true;
}

bool Self::begin()
{
  pinMode(_select, OUTPUT);
  digitalWrite(_select, HIGH);
 
  //  1 MHz max - datasheet page 8
  _spi_settings = SPISettings(_SPIspeed, MSBFIRST, SPI_MODE0);

  if (_hwSPI)
  {
    _mySPI->end();
    _mySPI->begin();
  }
  else
  {
    pinMode(_mosi,  OUTPUT);
    pinMode(_miso, INPUT);
    pinMode(_sclk,   OUTPUT);
    digitalWrite(_mosi, LOW);
    digitalWrite(_sclk,   LOW);
  }
  return true;
}

uint8_t Self::setVolume(volume_t lch, volume_t rch)
{
  if ((lch > 0) || (rch > 0) || (lch < -447) || (rch < -447))
  {
    _error = Muses72323_VALUE_ERROR;
    return false;
  }
  transfer(s_control_attenuation_l, volume_to_attenuation(lch));
  transfer(s_control_attenuation_r, volume_to_attenuation(rch));
  return true;
}

uint8_t Self::setGain(data_t intgain)
{
  if ((intgain > 0) || (intgain > 0))
  {
    _error = Muses72323_VALUE_ERROR;
    return false;
  }
  gain = (intgain << 12) | (intgain << 9);
  bitWrite(gain, s_state_bit_zero_crossing, false);
  transfer(s_control_gain, gain);
  return true;
}

void Self::mute()
{
  transfer(s_control_attenuation_l, 0);
  transfer(s_control_attenuation_r, 0);
}

void Self::setExternalClock(bool enabled)
{
  // 0 external, 1 internal
  bitWrite(states, s_state_external_clock, !enabled);
  transfer(s_control_states, states);
}

void Self::setZeroCrossingOn(bool enabled)
{
  // 0 is enabled, 1 is disabled
  bitWrite(gain, s_state_bit_zero_crossing, !enabled);
  transfer(s_control_gain, gain);
}

void Self::setLinkChannels(bool enabled)
{
  // 1 is enabled (linked channels), 0 is disabled
  bitWrite(gain, s_state_bit_gain, enabled);
  transfer(s_control_gain, gain);
}

void Self::transfer(address_t s_control_address, data_t data)
{
  word tmp;
  tmp = s_control_address | _chip_address;
  tmp = tmp | data;
  digitalWrite(_select, LOW);
  if (_hwSPI)
  {
    _mySPI->beginTransaction(_spi_settings);
    _mySPI->transfer(highByte(tmp));
    _mySPI->transfer(lowByte(tmp));
    _mySPI->endTransaction();
  }
  else
  {
    swSPI_transfer(highByte(tmp));
    swSPI_transfer(lowByte(tmp));
  }
  digitalWrite(_select, HIGH);
  SPI.endTransaction();
}

uint8_t  Self::swSPI_transfer(uint8_t value)
{
  uint8_t clk = _sclk;
  uint8_t dao = _mosi;
  uint8_t dai = _miso;

  uint8_t rv = 0;
  for (uint8_t mask = 0x80; mask > 0; mask >>= 1)
  {
    digitalWrite(dao, (value & mask) ? HIGH : LOW);
    digitalWrite(clk, HIGH);
    if (::digitalRead(dai) == HIGH) rv |= mask;
    digitalWrite(clk, LOW);
  }
  return rv;
}
