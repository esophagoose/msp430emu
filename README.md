MSP430 Emulator
===============

- Providing a complete software model of the MSP430 16-bit instruction set
- An interactive debugger for advanced development and in depth firmware/hardware analysis
- Peripherals include:
  - GPIO Ports (LEDs and other pins)
  - UART Serial Communication (via USCI Module)
  - PWM Servo Motor Control
  - Timer_A (in progress)
  - BCM+ (in progress)

  The project goal is to emulate all peripherals and devices on the TI MSP430 Launchpad starter kit, 
  to be able to run all firmware that would run on the physical device and programmatically test hardware inputs like    UART / other digital ports. The emulator is written in C/C++ and acts as an API to the MCU.
  
  Please contact rgeosits@live.esu.edu if you are interested in using this tool for educational or industrial purposes.
  Thank you!
  
--------------------------------------------------------------------------------------------------------------------------

- Build Instructions (CLI)
  - Install dependencies:
    - macOS (Homebrew): `brew install readline llvm`
    - Linux (Debian/Ubuntu): `./install_deps.sh`
  - Navigate to the root of the source tree
  - Run `make`

- User Instructions (CLI)
  - Run `./MSP430 path/to/firmware.elf`
  - `.bin` inputs are also supported: `./MSP430 path/to/firmware.bin`
  - Optional IPC stream (UNIX socket): `./MSP430 path/to/firmware.elf --ipc unix:/tmp/msp430.sock`
  - At the debugger prompt use `h` for commands, then `run`, `step`, `break`, etc.
  - Listen from Python: `python3 tools/gpio_listener.py --socket /tmp/msp430.sock`

- Documentation & Sample Programs:
  - Firmware samples: `430x2_firm/`

- TODO
  - Basic Clock Module / Timer  
  - Instructions
    - DADD (BCD math)
    - RETI (Return from Interrupt)
