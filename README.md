# MobileWirelessAndNetwork-ESP32 
# Report 01 Draft
## 1.0 Introduction

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

## 3.0 Programming and Implementation
### 3.1 LED Control via Push Button
#### 3.1.1 Implementation Code
`

`
### 3.2 Temperature and Humidity Data Collection with DHT11
#### 3.2.1 Implementation Code
`

`

## 4.0 Testing and Validation

## 5.0 Conclusion
