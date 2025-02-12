PREFIX	= arm-none-eabi
CC		= $(PREFIX)-gcc
OBJCOPY	= $(PREFIX)-objcopy
OBJDUMP	= $(PREFIX)-objdump

# MK_DIR: directory of this .mk file, not mkdir(1)
MK_DIR  = $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

# platform specific options
CFLAGS  += -mcpu=cortex-m3 -mthumb -DSTM32F1 
LDFLAGS += -mcpu=cortex-m3 -mthumb -lc -T$(MK_DIR)/libopencm3.ld -lopencm3_stm32f1 

# general optimization options
CFLAGS	+= -fno-common -ffunction-sections -fdata-sections
LDFLAGS	+= -static -nostartfiles -Wl,--gc-sections

OBJECTS = $(patsubst %.c,%.o,$(SOURCES))

all: $(TARGET).bin

$(TARGET).bin: $(TARGET)
	$(OBJCOPY) -Obinary $(TARGET) $(TARGET).bin

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

flash: $(TARGET).bin
	sudo $(MK_DIR)/stm32flash -w $(TARGET).bin -vg0 /dev/ttyUSB0

list: $(TARGET)
	$(OBJDUMP) -S $< > $(TARGET).list

clean:
	rm $(OBJECTS) $(TARGET) $(TARGET).bin $(TARGET).list 2> /dev/null || true
