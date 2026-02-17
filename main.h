/*
  MSP430 Emulator
  Copyright (C) 2022 Rudolf Geosits (rgeosits@live.esu.edu)

  "MSP430 Emulator" is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  "MSP430 Emulator" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

typedef struct Emulator Emulator;

typedef struct Cpu Cpu;
typedef struct Port_1 Port_1;
typedef struct Usci Usci;
typedef struct Bcm Bcm;
typedef struct Timer_a Timer_a;
typedef struct Status_reg Status_reg;

typedef struct Debugger Debugger;

void print_console(Emulator *emu, const char *buf);
void print_serial(Emulator *emu, char *buf);
void send_control(Emulator *emu, uint8_t opcode, void *data, size_t size);
bool ipc_configure(const char *spec, char *err, size_t err_len);
void ipc_close(void);
void ipc_emit_gpio(Emulator *emu, uint8_t port, uint8_t pin, uint8_t dir, uint8_t value);
void ipc_emit_uart_tx(Emulator *emu, uint8_t value);

enum {
    SERVO_MOTOR = 0x21,
};

#include "devices/peripherals/bcm.h"
#include "devices/peripherals/timer_a.h"
#include "devices/peripherals/port1.h"
#include "devices/peripherals/usci.h"
#include "devices/cpu/registers.h"
#include "devices/utilities.h"
#include "devices/memory/memspace.h"
#include "devices/cpu/decoder.h"
#include "debugger/debugger.h"
#include "debugger/register_display.h"
#include "debugger/disassembler.h"

struct Emulator
{
    Cpu *cpu;
    Debugger *debugger;
};
