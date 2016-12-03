#Makefile para compilar para la EDU-CIAA Baremetal usando las bibliotecas de LPCOpen
#Extraído de https://github.com/martinribelotta/ciaa_lpcopen_bare

APP=bla_app_lpc4337_baremetal

MODULES=lpc_chip_43xx lpc_board_ciaa_edu_4337 ../CMSIS_DSPLIB_SRC ../../bla
DEFINES=CORE_M4 __USE_LPCOPEN 

TARGET=$(APP).elf
SRC=$(foreach m, $(MODULES), $(wildcard $(m)/src/*.c)) $(wildcard $(APP)/src/*.c)
INCLUDES=$(foreach m, $(MODULES), -I$(m)/inc) -Ilpc_chip_43xx/inc/usbd/ $(foreach i, $(wildcard $(APP)/*/inc), -I$(i))
_DEFINES=$(foreach m, $(DEFINES), -D$(m))
OBJECTS=$(SRC:.c=.o) $(SRC_ASM:.S=.o)
DEPS=$(SRC:.c=.d)
LDSCRIPT=ldscript/ciaa_lpc4337.ld
LIBS=
ARCH_FLAGS= 

CFLAGS=$(ARCH_FLAGS) $(INCLUDES) $(_DEFINES) -O2 -nostartfiles -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fsingle-precision-constant -mcpu=cortex-m4  -Wdouble-promotion -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -pedantic -Wall
LDFLAGS=$(ARCH_FLAGS) -T$(LDSCRIPT) 
LDFLAGS+=-Wl,-gc-sections -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb
LDFLAGS+=-L../CMSIS_DSPLIB_SRC/lib -lcmsisdsp_cortexM4
all: $(TARGET)

CROSS=arm-none-eabi-
CC=$(CROSS)gcc
LD=$(CROSS)gcc
SIZE=$(CROSS)size
OBJCOPY=$(CROSS)objcopy
LIST=$(CROSS)objdump -xCedlSwz
GDB=$(CROSS)gdb
OOCD=openocd

ifeq ("$(origin V)", "command line")
BUILD_VERBOSE=$(V)
endif
ifndef BUILD_VERBOSE
BUILD_VERBOSE = 0
endif
BUILD_VERBOSE = 1
ifeq ($(BUILD_VERBOSE),0)
Q = @
else
Q =
endif

-include $(DEPS)

%.o: %.c
	@echo "CC $<"
	$(Q)$(CC) -MMD $(CFLAGS) -c -o $@ $<
	
%.o: %.S
	@echo "CC ASM $<"
	$(Q)$(CC) -MMD $(CFLAGS) -c -o $@ $<	

$(TARGET): $(OBJECTS) Makefile
	@echo "LD $@"
	$(Q)$(LD) -o $@ $(OBJECTS) $(LDFLAGS)
	$(Q)$(OBJCOPY) -v --output-target binary --remove-section=.text_Flash2 $@ $(APP).bin
	$(Q)$(OBJCOPY) -v --only-section=.text_Flash2 --output-target binary $@ $(APP)_bankB.bin
	$(Q)$(LIST) $@ > $(APP).lst
	$(Q)echo Executable SIZE:
	$(Q)$(SIZE) $@

.PHONY: clean debug openocd

openocd:
	$(Q)$(OOCD) -f ciaa-nxp.cfg

debug: $(TARGET)
	$(Q)$(GDB) $< -ex "target remote :3333" -ex "mon reset halt" -ex "load" -ex "continue"

_run: $(TARGET)
	$(Q)$(GDB) $< -batch -ex "target remote :3333" -ex "mon reset halt" -ex "load" -ex "mon reset run" -ex "quit"

run: $(TARGET)
	$(Q)$(OOCD) -f ciaa-nxp.cfg \
		-c "init" \
		-c "halt 0" \
		-c "flash write_image erase unlock $(APP).bin 0x1A000000 bin" \
		-c "reset run" \
		-c "shutdown"
	 rem -c "flash write_image erase unlock $(APP)_bankB.bin 0x1B000000 bin" \
	
		
erase:
	$(Q)$(OOCD) -f ciaa-nxp.cfg \
		-c "init" -c "halt 0" -c "flash erase_sector 0 0 last" -c "shutdown"

clean:
	@echo "CLEAN"
	C:\MinGW\msys\1.0\bin\rm.exe -fR $(OBJECTS) $(TARGET) $(DEPS) $(APP).lst
