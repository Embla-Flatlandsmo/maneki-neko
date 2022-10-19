# Maneki Neko

## Project structure
| Folder | Description |
| --- | --- |
| `code` | Software for the microcontroller unit |
| `production` | Models for 3D printing the parts |
| `schematics` | Images describing the way the electronics are wired |

## Hardware description
The project supports using both [nRF52840 DK](https://www.nordicsemi.com/Products/Development-hardware/nrf52840-dk) and [Adafruit ItsyBitsy nRF52840 Express](https://learn.adafruit.com/adafruit-itsybitsy-nrf52840-express).

### nRF 52840 DK
#### Wiring
| Pin | Name |
| --- | --- |
| P1.10 | Encoder 1 Line A |
| P1.11 | Encoder 1 Line B |
| P0.27 | Encoder 2 Line A |
| P0.26 | Encoder 2 Line B |
| P0.13 | Servo PWM output | 


### Adafruit ItsyBitsy nRF52840 Express
#### Wiring
| Pin | Name |
| --- | --- |
| 13 | Encoder 1 Line A |
| 12 | Encoder 1 Line B |
| 11 | Encoder 2 Line A |
| 10 | Encoder 2 Line B |
| 7 | Servo PWM output | 

## Software Description
This project uses the [nRF Connect SDK v2.0.2](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.0.2/nrf/index.html).
### Setup using the nRF Connect SDK Toolchain Manager
Prerequisite: [nRF Command Line Tools](https://www.nordicsemi.com/Products/Development-tools/nRF-Command-Line-Tools/Download?lang=en#infotabs)
1. **Install ncs v2.0.2**: Follow [nRF Connect SDK (ncs) setup tutorial](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.0.2/nrf/gs_assistant.html) to install the nRF Connect SDK (ncs) toolchain.
2. Make sure you are able to build and program a basic project following [the nRF Connect SDK building tutorial](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_programming.html)

3. **Clone this project into the ncs installation folder**: After installing nRF Connect SDK v2.0.2, open command prompt through the toolchain manager. Then, type `git clone https://github.com/Embla-Flatlandsmo/maneki-neko`

### First time setup for Adafruit ItsyBitsy nRF52840
Before building for the first time, you must copy the folder `code/boards/adafruit_itsybitsy_nrf52840` into the `boards/arm/` directory of your zephyr installation. 

> For example, if ncs is installed at `C:/ncs/v2.0.2` then you would copy the folder so it resides at `C:/ncs/v2.0.2/zephyr/boards/arm/adafruit_itsybitsy_nrf52840`

Before flashing, you must update the Adafruit UF2 bootloader [(see tutorial)](https://learn.adafruit.com/introducing-the-adafruit-nrf52840-feather/update-bootloader)

### Building and Flashing
Before building the project, make sure you are able to build and program a basic project following [the nRF Connect SDK building tutorial](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_programming.html).


> For both of the boards, follow the steps for [building in the command line](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.0.2/nrf/gs_programming.html#gs-programming-cmd).


#### Adafruit ItsyBitsy nRF52840 Express
1. Navigate to `maneki-neko/code` and build the program by calling `west build -b adafruit_itsybitsy_nrf52840`
2. Connect the ItsyBitsy to your host computer using USB
3. Tap the reset button twice quickly to enter bootloader mode
4. Flash the image by dragging and dropping the file `maneki-neko/code/build/zephyr/zephyr.uf2` into the `ITSY840BOOT` drive
5. The program should now run on your device

#### nRF52840 DK
1. Navigate to `maneki-neko/code` and build the program by calling `west build -b nrf52840dk_nrf52840`
2. Connect the nRF52840 DK to your computer using USB
3. Call `west flash`
4. The program should now run on your device