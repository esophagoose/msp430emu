all: MSP430

MSP430 : main.o utilities.o io.o registers.o memspace.o debugger.o disassembler.o \
	register_display.o decoder.o flag_handler.o formatI.o formatII.o formatIII.o \
	usci.o port1.o bcm.o timer_a.o

	g++ -o MSP430 main.o utilities.o io.o registers.o memspace.o debugger.o disassembler.o \
	register_display.o decoder.o flag_handler.o formatI.o formatII.o formatIII.o usci.o port1.o bcm.o timer_a.o -lreadline -lpthread

main.o : main.cpp
	g++ -c main.cpp

utilities.o : devices/utilities.c
	g++ -c devices/utilities.c

registers.o : devices/cpu/registers.c
	g++ -c devices/cpu/registers.c

memspace.o : devices/memory/memspace.c
	g++ -c devices/memory/memspace.c

debugger.o : debugger/debugger.c
	g++ -c debugger/debugger.c

io.o : debugger/io.c
	g++ -c debugger/io.c

disassembler.o : debugger/disassembler.c
	g++ -c debugger/disassembler.c

register_display.o : debugger/register_display.c
	g++ -c debugger/register_display.c

decoder.o : devices/cpu/decoder.c
	g++ -c devices/cpu/decoder.c

flag_handler.o : devices/cpu/flag_handler.c
	g++ -c devices/cpu/flag_handler.c

formatI.o : devices/cpu/formatI.c
	g++ -c devices/cpu/formatI.c

formatII.o : devices/cpu/formatII.c
	g++ -c devices/cpu/formatII.c

formatIII.o : devices/cpu/formatIII.c
	g++ -c devices/cpu/formatIII.c

bcm.o : devices/peripherals/bcm.c
	g++ -c devices/peripherals/bcm.c

timer_a.o : devices/peripherals/timer_a.c
	g++ -c devices/peripherals/timer_a.c

usci.o : devices/peripherals/usci.c
	g++ -c devices/peripherals/usci.c

port1.o : devices/peripherals/port1.c
	g++ -c devices/peripherals/port1.c

clean :
	rm -f main.o utilities.o io.o registers.o \
	memspace.o debugger.o disassembler.o \
	register_display.o decoder.o flag_handler.o formatI.o \
	formatII.o formatIII.o \
	usci.o port1.o bcm.o timer_a.o \
	*.bin *.tmp *.elf \
	MSP430;
