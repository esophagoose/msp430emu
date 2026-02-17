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

#include "main.h"

static bool has_suffix(const char *str, const char *suffix)
{
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > str_len)
    {
        return false;
    }

    return strncmp(str + (str_len - suffix_len), suffix, suffix_len) == 0;
}

static bool convert_elf_to_bin(const char *elf_path, const char *bin_path)
{
    const char *objcopy_cmds[] = {"llvm-objcopy", "gobjcopy", "objcopy"};
    char cmd[1024] = {0};
    size_t i;

    for (i = 0; i < (sizeof(objcopy_cmds) / sizeof(objcopy_cmds[0])); i++)
    {
        snprintf(cmd, sizeof(cmd),
                 "%s -I elf32-little -O binary \"%s\" \"%s\" >/dev/null 2>&1",
                 objcopy_cmds[i], elf_path, bin_path);

        if (system(cmd) == 0)
        {
            return true;
        }
    }

    return false;
}

int main(int argc, char *argv[])
{
    const char *input_firmware = NULL;
    const char *firmware_bin = NULL;
    const char *ipc_spec = NULL;
    char temp_bin_path[256] = {0};
    char ipc_err[256] = {0};
    bool created_temp_bin = false;
    int argi;

    for (argi = 1; argi < argc; argi++)
    {
        if (strcmp(argv[argi], "--ipc") == 0)
        {
            if (argi + 1 >= argc)
            {
                fprintf(stderr, "Usage: %s <firmware.elf|firmware.bin> [--ipc unix:/tmp/msp430.sock]\n", argv[0]);
                return 1;
            }

            ipc_spec = argv[++argi];
        }
        else if (strncmp(argv[argi], "--ipc=", 6) == 0)
        {
            ipc_spec = argv[argi] + 6;
        }
        else if (argv[argi][0] == '-')
        {
            fprintf(stderr, "Unknown option: %s\n", argv[argi]);
            fprintf(stderr, "Usage: %s <firmware.elf|firmware.bin> [--ipc unix:/tmp/msp430.sock]\n", argv[0]);
            return 1;
        }
        else if (input_firmware == NULL)
        {
            input_firmware = argv[argi];
        }
        else
        {
            fprintf(stderr, "Unexpected argument: %s\n", argv[argi]);
            fprintf(stderr, "Usage: %s <firmware.elf|firmware.bin> [--ipc unix:/tmp/msp430.sock]\n", argv[0]);
            return 1;
        }
    }

    if (input_firmware == NULL)
    {
        fprintf(stderr, "Usage: %s <firmware.elf|firmware.bin> [--ipc unix:/tmp/msp430.sock]\n", argv[0]);
        return 1;
    }

    if (has_suffix(input_firmware, ".elf"))
    {
        snprintf(temp_bin_path, sizeof(temp_bin_path), "/tmp/msp430_cli_%d.bin", getpid());

        if (!convert_elf_to_bin(input_firmware, temp_bin_path))
        {
            fprintf(stderr,
                    "Failed to convert ELF to BIN. Install one of: llvm-objcopy, gobjcopy, or objcopy.\n");
            return 1;
        }

        firmware_bin = temp_bin_path;
        created_temp_bin = true;
    }
    else
    {
        firmware_bin = input_firmware;
    }

    Emulator *emu = (Emulator *) calloc( 1, sizeof(Emulator) );
    Cpu *cpu = NULL; Debugger *deb = NULL;

    emu->cpu       = (Cpu *) calloc(1, sizeof(Cpu));
    emu->cpu->bcm  = (Bcm *) calloc(1, sizeof(Bcm));
    emu->cpu->timer_a  = (Timer_a *) calloc(1, sizeof(Timer_a));
    emu->cpu->p1   = (Port_1 *) calloc(1, sizeof(Port_1));
    emu->cpu->usci = (Usci *) calloc(1, sizeof(Usci));  

    emu->debugger  = (Debugger *) calloc(1, sizeof(Debugger));
    setup_debugger(emu);

    cpu = emu->cpu;
    deb = emu->debugger;

    if (ipc_spec != NULL)
    {
        if (!ipc_configure(ipc_spec, ipc_err, sizeof(ipc_err)))
        {
            fprintf(stderr, "Failed to initialize IPC: %s\n", ipc_err[0] ? ipc_err : "unknown error");
            free(cpu->timer_a);
            free(cpu->bcm);
            free(cpu->p1);
            free(cpu->usci);
            free(cpu);
            free(deb);
            free(emu);
            if (created_temp_bin)
            {
                unlink(temp_bin_path);
            }
            return 1;
        }

        printf("IPC ready on %s\n", ipc_spec);
    }

    register_signal(SIGINT); // Register callback for CONTROL-C
  
    initialize_msp_memspace();
    initialize_msp_registers(emu);  

    setup_bcm(emu);
    setup_timer_a(emu);
    setup_port_1(emu);
    setup_usci(emu);
  
    load_bootloader(0x0C00);
    load_firmware(emu, (char *)firmware_bin, 0xC000);
    
    puts(" [MSP430 Emulator]");
    puts(" Type 'h' for debugger options.\n");

    // display first round of registers
    display_registers(emu);
    disassemble(emu, cpu->pc, 1);
    update_register_display(emu);

    // Fetch-Decode-Execute Cycle (run machine)
    while (!deb->quit)
    {
        // Handle debugger when CPU is not running
        if (!cpu->running)
        {
            if (deb->debug_mode)
            {
                char *line = readline("\n>> ");

                if (line == NULL)
                {
                    deb->quit = true;
                    break;
                }

                if (strlen(line) > 0)
                {
                    add_history(line);
                    exec_cmd(emu, line, (int)strlen(line));
                }

                free(line);
            }
            else
            {
                usleep(10000);
            }

            continue;
        }

        // Handle Breakpoints
        handle_breakpoints(emu);
        
        // Instruction Decoder
        decode(emu, fetch(emu), EXECUTE); 
        
        // Handle Peripherals
        handle_bcm(emu);
        handle_timer_a(emu);
        handle_port_1(emu);
        handle_usci(emu);

        // Average of 4 cycles per instruction
        mclk_wait_cycles(emu, 4);
    }

    uninitialize_msp_memspace();
    free(cpu->timer_a);
    free(cpu->bcm);
    free(cpu->p1);
    free(cpu->usci);
    free(cpu);
    free(deb);
    free(emu);
    ipc_close();

    if (created_temp_bin)
    {
        unlink(temp_bin_path);
    }

    return 0;
}
