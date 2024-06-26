# default target when "make" is run w/o arguments

.PHONY: all clean CAN-RS485-install

# device specific naming and (sub)paths
DEVICE=same51j20a
CPU=Cortex-M4
BOARD=microchip_same51_curiosity_nano
PACK=E51-pack

# standard compiling setup
CC=arm-none-eabi-gcc
DEVICE_UPPER=$(shell echo $(DEVICE) | tr  '[:lower:]' '[:upper:]')
BUILDDIR=build
SRCDIR=src
TARGET-CAN-RS485=$(BUILDDIR)/CAN-RS485.elf
TARGET-SENSOR=$(BUILDDIR)/sensor.elf
TARGET-WIFI-CAN=$(BUILDDIR)/WIFI-CAN.elf

INCLUDE_PATHS=-I$(PACK)/include -ICore/include -Iinclude
ASFLAGS=-mthumb -mcpu=$(CPU) -D__$(DEVICE_UPPER)__ -O1 -ffunction-sections -Wall
CFLAGS=-x c -mthumb -mcpu=$(CPU) -D__$(DEVICE_UPPER)__ -O1 -ffunction-sections -Wall -c -std=gnu99
LDFLAGS=-Wl,--start-group -lm  -Wl,--end-group -Wl,--gc-sections -mthumb -mcpu=$(CPU) -T$(PACK)/gcc/gcc/$(DEVICE)_flash.ld

SYS_OBJS=$(PACK)/gcc/system_$(DEVICE).o $(PACK)/gcc/gcc/startup_$(DEVICE).o

SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

all: $(TARGET-SENSOR) $(TARGET-CAN-RS485) $(TARGET-WIFI-CAN)

debug: CFLAGS += -g
debug: clean all
release: CFLAGS += -DNDEBUG
release: clean all

.PRECIOUS: %.o

# build all sources needed by the project
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDE_PATHS) $^ -o $@

$(BUILDDIR)/CAN-RS485.o: CAN-RS485.c
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDE_PATHS) $^ -o $@

$(BUILDDIR)/sensor.o: sensor.c
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDE_PATHS) $^ -o $@

$(BUILDDIR)/WIFI-CAN.o: WIFI-CAN.c
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDE_PATHS) $^ -o $@

# build the executable file
$(TARGET-CAN-RS485): $(OBJS) $(SYS_OBJS) build/CAN-RS485.o
	mkdir -p build
	$(CC) -o $@ $(LDFLAGS) $^
	arm-none-eabi-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature  $@ $(patsubst %.elf,%.hex,$@)
	arm-none-eabi-objdump -h -S $@ > $(patsubst %.elf,%.lss,$@)
	arm-none-eabi-size $@

$(TARGET-SENSOR): $(OBJS) $(SYS_OBJS) build/sensor.o
	mkdir -p build
	$(CC) -o $@ $(LDFLAGS) $^
	arm-none-eabi-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature  $@ $(patsubst %.elf,%.hex,$@)
	arm-none-eabi-objdump -h -S $@ > $(patsubst %.elf,%.lss,$@)
	arm-none-eabi-size $@

$(TARGET-WIFI-CAN): $(OBJS) $(SYS_OBJS) build/WIFI-CAN.o
	mkdir -p build
	$(CC) -o $@ $(LDFLAGS) $^
	arm-none-eabi-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature  $@ $(patsubst %.elf,%.hex,$@)
	arm-none-eabi-objdump -h -S $@ > $(patsubst %.elf,%.lss,$@)
	arm-none-eabi-size $@

clean:
	rm -rf build/

# put the executable on the device
CAN-RS485-install: $(TARGET-CAN-RS485)
	openocd -f board/$(BOARD).cfg -c "program $< verify reset exit"

sensor-install: $(TARGET-SENSOR)
	openocd -f board/$(BOARD).cfg -c "program $< verify reset exit"

WIFI-CAN-install: $(TARGET-WIFI-CAN)
	openocd -f board/$(BOARD).cfg -c "program $< verify reset exit"
