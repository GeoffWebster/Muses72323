# Muses 72323

Arduino library for communicating with the Muses 72323 audio chip.
The data sheets can be found [here](https://www.nisshinbo-microdevices.co.jp/en/pdf/datasheet/MUSES72323_E.pdf) (pdf).

## Example

```c++
#include <Muses72323.h>

// The address wired into the muses chip (usually 0).
static const byte MUSES_ADDRESS = 0;

static Muses72323 Muses(MUSES_ADDRESS);
static Muses72323::volume_t CurrentVolume = -20;

void setup()
{
  // Initialize muses (SPI, pin modes)...
  Muses.begin();

  // Muses initially starts in a muted state, set a volume to enable sound.
  Muses.setVolume(CurrentVolume,CurrentVolume);

  // These are the default states and could be removed...
  Muses.setZeroCrossing(true);     // Enable/Disable zero crossing.
  Muses.setAttenuationLink(false); // Channel controls independant L/R attenuation channel.
}

void loop()
{
  CurrentVolume -= 1;
  if (CurrentVolume < -447)
  {
    CurrentVolume = 0;
  }

  Muses.setVolume(CurrentVolume, CurrentVolume);
  delay(10);
}

```

## Problems

Please post any problems on the [Issues](https://github.com/GeoffWebster/Muses72323) page.

## License

Please read over the LICENSE file included in the project.
