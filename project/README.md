# Rotating Platform

## Hardware description

## Software Description
This project uses the [nRF Connect SDK v2.0.2](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.0.2/nrf/index.html).
### Setup using the nRF Connect SDK Toolchain Manager
Prerequisite: [nRF Command Line Tools](https://www.nordicsemi.com/Products/Development-tools/nRF-Command-Line-Tools/Download?lang=en#infotabs)
1. **Install ncs v2.0.2**: Follow [nRF Connect SDK setup tutorial](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.0.2/nrf/gs_assistant.html) to install the nRF Connect SDK (ncs) toolchain.
2. **Clone this project into the ncs installation folder**: After installing nRF Connect SDK v2.0.2, open command prompt through the toolchain manager. Then, type `git clone https://github.com/Embla-Flatlandsmo/rotating-platform`
 
### Building and Flashing
Follow the steps for [building in the command line](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.0.2/nrf/gs_programming.html#gs-programming-cmd). For example, if ncs is installed in `C:/ncs/v2.0.2` and the project is cloned to the root folder, then from `C:/ncs/v2.0.2/rotating-platform/project` call `west build -b nrf52840dk_nrf52840`.


To flash ensure that the nRF52840 DK is connected to the PC through USB and that it is turned on. From the same folder you called `west build`,  call `west flash`.