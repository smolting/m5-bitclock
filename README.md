# M5-bitclock

A tiny **wireless bitcoin metrics ticker** that will last for days on battery

![m5-bitclock](docs/pictures/m5-bitclock.png)

## Feature Overview

* **Metrics** - block height, price, mempool size, next-block fee)
* **Power efficient** - epaper display (EPD) allows efficient use of the battery; unit lasts for days on a single charge!

## Hardware Requirements
* **[M5 Stack CoreInk Development Kit (~$35 USD)](https://shop.m5stack.com/products/m5stack-esp32-core-ink-development-kit1-54-elnk-display)** - CoreInk is an IoT prototyping device with WiFi, Bluetooth, a real-time clock, and a 1.5" electronic paper display

## Software Requirements
To flash the software to the CoreInk, you will need:

* **[Arduino IDE](https://www.arduino.cc/en/Main/Software)**

## Installation Instructions (estimated time ~30 minutes)

(Navigate [to the M5 CoreInk Arduino Quick Start Guide](https://docs.m5stack.com/en/quick_start/coreink/arduino) for detailed environment setup instructions)

1. Install Arduino IDE (see software requirements)

2. Open Arduino IDE, and configure the boards manager:
   
   * In the Arduino IDE, navigate to **File Menu** -> **Preferences**
  
   * Locate the **Additional Boards Manager** setting and add the following URL to it: `https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json` 
   * ![additional-boards](docs/pictures/preferences-m5-arduino.png)
  
   * Navigate to **Tools Menu** -> **Board: (XXX)** -> **Boards Manager** 
   * Search for "**`M5Stack`**" (by **M5Stack official**) and install the board configuration 
   * ![boards-manager](docs/pictures/boards-manager-m5stack.png)
   * Set the board configuration: **Tools Menu** -> **Board: (XXX)** -> **M5Stack Arduino** -> **M5Stack-CoreInk** 
   * ![boards-menu](docs/pictures/boards-menu.png)
  
3. Install the **M5-CoreInk** Libraries:
   * **Sketch Menu** -> **Include Library** -> **Manage Libraries** 
   * Search for "**`CoreInk`**" and install the **M5-CoreInk** library (by Hades) 
   * ![libraries-coreink](docs/pictures/library-manager-coreink.png)
   * Search for "**`M5 Stack`**" and install the **M5Stack** library (by M5) 
   * ![libraries-m5stack](docs/pictures/library-manager-m5stack.png)

4. Download and extract the latest release of this source from [here](https://github.com/smolting/m5-bitclock/releases)
   

5. In Arduino IDE, open m5-bitclock.ino sketch in the directory you extracted from the release archive (**File** -> **Open**)

6. The Arduino IDE will display a tab for every source file. Locate the tab for `preferences.c`, and in it, change the SSID and password values to reflect the WiFi network you would like the m5-bitclock to connect to:
   ```
   const WifiConfiguration WIFI_CONFIGURATION = { "MyHomeWifiRouter", "p@55w0rd" };
   ```

7. Connect the M5 CoreInk to a USB port, and configure the correct USB port:
   * **Tools Menu** -> **Port: (XXX)** -> **dev/ttyUSB0** -> **M5Stack-CoreInk**
   * ![ports-menu](docs/pictures/port-menu.png)

8. Upload the software:
   * Using the menu item: **Sketch Menu** -> **Upload** 
   * ![menu-upload](docs/pictures/sketch-upload.png)
   * or with the **Upload** button: 
   * ![upload-button](docs/pictures/upload-button.png)

## Contributing

See **Installation Instructions** for development environment setup

