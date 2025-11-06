# Ioctl-Message-Kernel-Driver
A Windows kernel driver in C with user-mode client demonstrating IOCTL communication.


## Overview

This project demonstrates Windows kernel driver development with:

- **Kernel Driver (`MyDriver.c`)**: Creates a device object `\Device\AmerDriver` and handles IOCTL requests
- **User-Mode Client (`client_for_driver.c`)**: Sends messages to the driver via `DeviceIoControl`

## Build Instructions

### Prerequisites

- Windows Driver Kit (WDK)
- Visual Studio with C/C++ support
- Windows SDK/WDK
- Administrator privileges for testing
- Windows 10/11 VM

### Runtime Library Configuration

**Important**: Set Runtime Library to "Multi-threaded (/MT)" for static linking:
- Project Properties → C/C++ → Code Generation → Runtime Library
- Enable Test Signing on VM:

      bcdedit /set testsigning on
