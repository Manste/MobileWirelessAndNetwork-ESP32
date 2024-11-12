# MobileWirelessAndNetwork-ESP32 - Report 01 Draft
## 1.0 Introduction
The rapid adoption of Internet of Things (IoT) has highlighted the need for efficient microcontroller-based solutions capable of supporting small-scale sensing, control, and data exchange systems. This report covers hands-on work with the ESP32 microcontroller to create a basic IoT setup, including LED control via a button and environmental sensing through a DHT11 temperature and humidity sensor. The report encompasses the hardware configuration, implementation of core functionalities, and validation methods employed in the setup.

## 2.0 System Setup and Hardware Configuration

### 2.1 ESP32 Overview and Selection
ESP32 is a series of low-cost, low-power system-on-chip microcontrollers with integrated Wi-Fi and dual-mode Bluetooth. The ESP32 series employs either a Tensilica Xtensa LX6 microprocessor in both dual-core and single-core variations, an Xtensa LX7 dual-core microprocessor, or a single-core RISC-V microprocessor and includes built-in antenna switches, RF balun, power amplifier, low-noise receive amplifier, filters, and power-management modules. These features make it ideal for IoT applications that require network connectivity, sensor integration, and user interaction.

### 2.2 Specification Overview
1. **Processors:** CPU: Xtensa dual-core (or single-core) 32-bit LX6 microprocessor, operating at 160 or 240 MHz and performing at up to 600 DMIPS; Ultra low power (ULP) co-processor.
2. **Memory:** 520 KiB SRAM.
3. **Wireless connectivity:** Wi-Fi: 802.11 b/g/n; Bluetooth: v4.2 BR/EDR and BLE (shares the radio with Wi-Fi)
4. **Peripheral interfaces:** 12-bit SAR ADC up to 18 channels; 2 × 8-bit DACs; 10 × touch sensors (capacitive sensing GPIOs); 4 × SPI; 2 × I²S interfaces; 2 × I²C interfaces; 3 × UART; SD/SDIO/CE-ATA/MMC/eMMC host controller; SDIO/SPI slave controller; Ethernet MAC interface with dedicated DMA and IEEE 1588 Precision Time Protocol support; CAN bus 2.0; Infrared remote controller (TX/RX, up to 8 channels); Motor PWM; LED PWM (up to 16 channels); Hall effect sensor; Ultra low power analog pre-amplifier.
5. **Security:** `IEEE 802.11 standard security features all supported, including WFA, WPA/WPA2 and WAPI; Secure boot; Flash encryption; 1024-bit OTP, up to 768-bit for customers.`
6. **Cryptographic hardware acceleration:** `AES, SHA-2, RSA, elliptic curve cryptography (ECC), random number generator (RNG).`
7. **Power management:** Internal low-dropout regulator; Individual power domain for RTC; 5μA deep sleep current; Wake up from GPIO interrupt, timer, ADC measurements, capacitive touch sensor interrupt.

### 2.3 Hardware Assembly and Circuit Design
The components were connected as given in the schematic (index 1) and described below:
1. **Light Emitting Diodes (LEDs):** The red LED was connected to GPIO32 and the white LED to GPIO33, each in series with a 220Ω resistor to limit current. These resistors prevented excessive current from damaging the LEDs, ensuring safe operation with the 3.3V GPIO output from the ESP32.
2. **Push Button:** The push button was wired to GPIO27, configured with an internal pull-up resistor (enabled via INPUT_PULLUP in code). This internal pull-up reduces the need for additional external components. However, an external resistor was connected to further stabilize the circuit, minimizing noise and unintended toggling due to electromagnetic interference.
3. **DHT11 Sensor:** The DHT11 sensor is connected to GPIO26 for data transmission, with its power and ground lines connected to the ESP32’s 3.3V and GND, respectively. A 4.7kΩ pull-up resistor was included between the sensor's data and power lines. This resistor stabilizes data transmission, preventing signal degradation and ensuring reliable communication between the ESP32 and the sensor.

## 3.0 Programming and Implementation
Using the code provided (index 2), the core features of controlling LEDs and monitoring environmental conditions with the DHT11 sensor were implemented.

### 3.1 LED Control via Push Button
The red LED is programmed to toggle on and off with each press of the push button, utilizing debounce logic to detect a state change. This involves comparing the button state with its previous state, allowing toggling only when a press is newly detected.
To be precise, we implemented this by utilizing an if-else condition within the void loop function. This approach activates the appropriate response based on the button's state, which can be either high or low. Since the button is configured with a pull-up resistor, a high state indicates that the button is not pressed, while a low state means that the button is pressed.
The white LED pin is reserved for additional functionalities or extensions.

### 3.2 Temperature and Humidity Data Collection with DHT11
The DHT11 sensor provided both humidity and temperature data via a single data line connected to the ESP32. The `DHT.h` library was used to simplify data acquisition, providing a straightforward API to retrieve temperature and humidity values. Temperature was read in Celsius, and humidity was recorded as a percentage, with each value displayed in the Serial Monitor. Error handling for sensor reads ensured that invalid data (resulting from sensor noise or connection issues) were flagged.

### 3.3 Data Output
Data collected from the DHT11 sensor are displayed on the Serial Monitor in real time, providing insight into the immediate environment. Temperature and humidity readings are updated regularly, and error messages to signal a sensor readings fail.

## 4.0 Testing and Validation
To install the setup environment, you have to:
* Install the ESP32 Board in Arduino IDE. The description of the steps to do that are available [here](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/).
* Install the library necessary to get data from the DHT sensor. Open your Arduino IDE and go to Sketch > Include Library > Manage Libraries. The Library Manager should open. Search for “DHT” on the Search box and install the DHT library from Adafruit.

### 4.1 LED and Button Test
To validate LED functionality, the push button was pressed multiple times to confirm toggling behavior. Debouncing was verified by ensuring only one state change per press.

### 4.2 Temperature and Humidity Test
Humidity and temperature data were tested under varying environmental conditions. Inconsistent or failed readings were addressed by reviewing connections and ensuring sufficient power supply to the sensor.

### 4.3 Troubleshooting Common Issues
Potential issues such as incorrect wiring, insufficient grounding, or code errors were systematically debugged. Notably;
1. LED not toggling: Fixed by verifying GPIO pin assignment and ensuring proper debouncing.
2. Sensor read errors: Resolved by checking sensor connections and verifying the DHT11 library.
3. Connectivity problems: We faced some connectivity problems with the given board. To be precise, the Arduino IDE did not recognize our device. After some research, we found a solution, which was to use an additional driver.

## 5.0 Conclusion
The ESP32 setup provided practical experience with IoT concepts, sensor integration, and microcontroller programming. The successful implementation of the LED control and environmental sensing builds a foundational understanding of IoT applications. This groundwork prepares for the next stages, including MQTT implementation and security strategy design, further expanding the IoT system’s capabilities.

## 6.0 References

## 7.0 Indexes
