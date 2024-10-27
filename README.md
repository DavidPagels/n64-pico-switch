# N64/Gamecube -> Raspberry Pi Pico -> Nintendo Switch

## Table of Contents

- [Description](#description)
- [Features](#features)
- [Inspiration](#inspiration)
- [Bluetooth](#bluetooth)
- [Setup](#setup)
- [3D Printed Enclosure/Plug](#3d-printed-enclosureplug)
- [Building with Docker](#building-with-docker)
- [Credits](#credits)

---

## Description

This code allows a Raspberry Pi Pico to control a Nintendo Switch using button and joystick inputs from an original N64 or Gamecube controller. This is a spiritual successor to my [N64-Arduino-Switch](https://github.com/DavidPagels/n64-arduino-switch) project.

## Features

Here's a list of some of the larger features of this project

- Connects to a Nintendo Switch via USB or Bluetooth.
- Also works with any platform that supports Nintendo Switch Pro controllers.
- Auto-detects whether the controller is N64 or Gamecube.
- Dynamically scales each axis of each analog joystick to account for reduced joystick range on old controllers.
- L + R + Start are mapped to the Switch home button.
- Supports rumble on Gamecube controllers and N64 controllers with Rumble Paks.

## Inspiration

It's been a year since I published the [N64-Arduino-Switch](https://github.com/DavidPagels/n64-arduino-switch) project. Since then, the wireless N64 Nintendo Switch controllers have still been largely out of stock and scalped. One of the pieces of feedback from the last project is that it looked intimidating to setup. I've since discovered the Raspberry Pi Pico and decided to redo this project for the Pico. This has 3 advantages over the Arduino project:
  
- Programming involves plugging in a Micro USB cable while holding a button, dragging and dropping a file, and you're done! No more installing a Device Firmware Update (DFU) programmer, installing extra libraries, or shorting pins to get the device into a DFU mode.
- The Pico only costs $4 instead of $10 for an off-brand, slower Arduino!
- For 2 more dollars than a base Pico, the Pico W microcontroller supports Bluetooth!

Also, when playing through The Legend of Zelda: Ocarina of Time last year on the Arduino version, it was annoying not having a way to go to the Switch's Home screen using the N64 controller. Since I didn't rely on a library to handle N64 controller communication this time, I found out that there is a special 'Reset' bit that's set when L + R + Start are held at the same time. This project maps that input to the Home button on the Switch!

## Bluetooth

This is my first project with Bluetooth, so the current version requires you to connect to a Switch in the 'Change Grip/Order' menu. If the Pico becomes disconnected, it may also have to be power cycled to get it to pair with the Switch again. I hope to improve the project so you only have to do this the first time you connect to a Switch, and so it automatically reconnects without power cycling. Also, while the Pico wirelessly connects to the Switch, it will still need to be powered via USB (any USB port - a battery bank should work).

## Setup

To program the Raspberry Pi Pico

1. Hold down the boot sel button on the Pico, and while holding, plug it into your PC via Micro USB cable.
2. Open the Pico in your File Explorer.
3. Download the .uf2 file from [this repository's most recent release](https://github.com/davidpagels/retro-pico-switch/releases).
4. Drag and drop the .uf2 file to the Pico folder from step 2.
5. Done!

Once these steps are done, [wire up your N64 controller](https://github.com/pothos/arduino-n64-controller-library/blob/master/README.md#wireing) or [your Gamecube controller](https://simplecontrollers.com/blogs/resources/gamecube-protocol) (pins 1 & 4 only required for rumble support). The pre-built .uf2 file assumes the data pin is on the Pico's [GP18 pin](https://datasheets.raspberrypi.com/pico/Pico-R3-A4-Pinout.pdf). A 1k pullup resistor should also be wired between 3.3v and your data pin to work properly.

## 3D Printed Enclosure/Plug

I've also designed a small enclosure to house the Pico and act as a plug for a switch controller! More details and files are available [here](https://www.thingiverse.com/thing:5823446)

<a href="https://www.thingiverse.com/thing:5823446">
  <img width="400" src="resources/N64%20Male%20Connector.jpg"/>
  <img width="400" src="resources/Pico%20Enclosure.jpg"/>
</a>

## Building with Docker

Having docker installed you can build this project yourself by simply running
the command:

```
docker build --output=./build .
```

The docker file in this project will install all necessary dependencies and run
the commands necessary to build this project. In the end the generated binary is
exported to the output folder of the command: "./build"

## Credits

This project would have taken a lot longer without the work done by everyone credited in the original [N64-Arduino-Switch](https://github.com/DavidPagels/n64-arduino-switch) project along with the creators and contributors of [TinyUsb](https://github.com/hathach/tinyusb), the [GP2040-CE](https://github.com/OpenStickCommunity/GP2040-CE) project, and the [MPG](https://github.com/OpenStickCommunity/MPG) project. Figuring out how to interface via Bluetooth also took me 1.5 months of work in my off time and would have taken even longer without the work done by Brikwerk on the [nxbt](https://github.com/Brikwerk/nxbt) project. If you're into developing this sort of stuff, definitely check their stuff out!
