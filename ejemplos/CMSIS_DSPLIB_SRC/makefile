CMSIS_DSP_LIB=lib/libcmsisdsp_cortexM4.a
CMSIS_DSP_LIB_PC=lib/libcmsisdsp_x86.a
CMSIS_DSP_SRC_ASM=src/TransformFunctions/arm_bitreversal2.S
CMSIS_DSP_SRC=$(wildcard src/*/*.c)
CMSIS_DSP_OBJ=$(CMSIS_DSP_SRC:.c=.o) $(CMSIS_DSP_SRC_ASM:.S=.o)
CMSIS_DSP_OBJ_PC=$(CMSIS_DSP_SRC:.c=.o_pc) arm_bitreversal_32.o_pc
CMSIS_DSP_INC=inc/

DEFINES=CORE_M4 __USE_LPCOPEN __FPU_PRESENT
_DEFINES=$(foreach m, $(DEFINES), -D$(m))
DEFINES_PC = ARM_MATH_CM0_FAMILY
_DEFINES_PC=$(foreach m, $(DEFINES_PC), -D$(m))

CROSS=arm-none-eabi-
CC=$(CROSS)gcc
LD=$(CROSS)gcc
AR=$(CROSS)ar

CFLAGS= -I$(CMSIS_DSP_INC) $(_DEFINES) -O2 -nostartfiles -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fsingle-precision-constant -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wno-implicit-function-declaration
CFLAGS_PC =  -I$(CMSIS_DSP_INC) $(_DEFINES_PC) -O2 -Wall -Wextra -Wno-implicit-function-declaration -Wall
 
all: $(CMSIS_DSP_LIB) $(CMSIS_DSP_LIB_PC)

$(CMSIS_DSP_LIB): $(CMSIS_DSP_OBJ)
	$(AR) rcs $@ $^
	
$(CMSIS_DSP_LIB_PC): $(CMSIS_DSP_OBJ_PC)
	ar rcs $@ $^	

%.o: %.c
	@echo "CC $<"
	$(CC) -MMD $(CFLAGS) -c -o $@ $<
		
%.o: %.s
	@echo "CC ASM $<"
	$(CC) -MMD $(CFLAGS) -c -o $@ $<	
	
%.o_pc: %.c
	@echo "CC $<"
	gcc -MMD $(CFLAGS_PC) -c -o $@ $<
		
