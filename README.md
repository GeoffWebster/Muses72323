# Muses 72323

Arduino library for communicating with the Muses 72323 audio chip.
Library is based on MUSES72320 library originally created by Christoffer Hjalmarsson.
Changes include:
* Increase clock speed to align with MUSES72323 data sheet of 1MHz (- 200KHz margin)
* Amendment of volume_to_attenuation routine to align with MUSES72323 settings
* Implement setGain function to provide option to adjust gain section between 0dB and +21dB in 3dB steps
* Amendment of hardware and software constructs plus software transfer to provide support to running more than one MUSES72323 seperately (as for use within a fully balanced preamplifier)
* Example amended to show each method for creating software SPI, hardware SPI with default chip address and hardware SPI with custon address.

The data sheets can be found [here](https://www.nisshinbo-microdevices.co.jp/en/pdf/datasheet/MUSES72323_E.pdf) (pdf).

## Example

```c++
#include <Arduino.h>
#include "Muses72323.h" // Hardware-specific library
#include <SPI.h> // Include SPI library for SPIClass
// define Muses72323 custom SPI pins
#define MUSES_SCLK 18
#define MUSES_MISO 19
#define MUSES_MOSI 23
#define MUSES_SELECT_L 16
#define MUSES_SELECT_R 17
// define Muses72323 hardware SPI pins
#define MUSES_HARDWARE_SELECT_L 16 // Hardware SPI select pin for left channel
#define MUSES_HARDWARE_SELECT_R 17 // Hardware SPI select pin for right channel
#define MUSES_ADDRESS 0x00 // Default chip address for Muses72323
// preAmp constructs
// Using software SPI
//Muses72323 Muses_L(MUSES_SCLK, MUSES_MISO, MUSES_MOSI, MUSES_SELECT_L, MUSES_ADDRESS); // Using software SPI
//Muses72323 Muses_R(MUSES_SCLK, MUSES_MISO, MUSES_MOSI, MUSES_SELECT_R, MUSES_ADDRESS); // Using software SPI
// Using hardware SPI
Muses72323 Muses_L(MUSES_HARDWARE_SELECT_L, &SPI); // Using hardware SPI and default address
Muses72323 Muses_R(MUSES_HARDWARE_SELECT_R, MUSES_ADDRESS, &SPI); // Using hardware SPI and custom address
void setup()
{
Serial.begin(115200);
// Initialize Muses chips
Muses_R.begin();
Muses_L.begin();
// Configure Muses chips
Muses_L.setExternalClock(false); // must be set!
Muses_R.setExternalClock(false); // must be set!
Muses_L.setZeroCrossingOn(true);
Muses_R.setZeroCrossingOn(true);
}
void loop()
{
for (int volume = -447; volume < 0; volume++)
{
Muses_L.setVolume(volume, volume);
Muses_R.setVolume(volume, volume);
}
}

```

## Problems

Please post any problems on the [Issues](https://github.com/GeoffWebster/Muses72323) page.

## License

Please read over the LICENSE file included in the project.
