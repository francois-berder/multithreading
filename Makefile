CC := arm-none-eabi-gcc
CP := arm-none-eabi-objcopy
OD := arm-none-eabi-objdump

CONFIG ?= release

ifndef BOARD
$(error BOARD is not set)
endif

BUILDDIR := build
OBJDIR := $(BUILDDIR)/$(BOARD)/$(CONFIG)/obj
DEPDIR := $(BUILDDIR)/$(BOARD)/$(CONFIG)/dep
BINDIR := $(BUILDDIR)/$(BOARD)/$(CONFIG)/bin

TARGET := multithreading

SRCS := main.c scheduler.c startup.c
SRCS := $(SRCS:%=src/%)
OBJS := $(SRCS:%.c=$(OBJDIR)/%.o)
DEPS := $(SRCS:%.c=$(DEPDIR)/%.d)

include boards/$(BOARD)/Makefile.board

CFLAGS += -mthumb -mthumb-interwork
LDFLAGS += -mthumb -mthumb-interwork

CFLAGS += -Wall -Wextra -std=c11
CFLAGS += -ffreestanding
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -I CMSIS/Include -I src
DEPFLAGS = -MMD -MP -MF $(@:$(OBJDIR)/%.o=$(DEPDIR)/%.d)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BINDIR)/$(TARGET).map
LDFLAGS += -specs=nosys.specs
LDFLAGS += -nostartfiles

ifeq ($(CONFIG),release)
CFLAGS += -O2 -fno-delete-null-pointer-checks
LDFLAGS += -O2 -fno-delete-null-pointer-checks
else ifeq ($(CONFIG),debug)
CFLAGS += -ggdb -g3
LDFLAGS += -ggdb -g3
endif

$(BINDIR)/$(TARGET).hex: $(BINDIR)/$(TARGET).elf
	$(CP) -O ihex $< $@

$(BINDIR)/$(TARGET).elf: $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $^ -o $@
	$(OD) -DwS $@ > $(BINDIR)/$(TARGET).dis

$(OBJDIR)/%.o: %.c
	@mkdir -p $(OBJDIR)/$(<D)
	@mkdir -p $(DEPDIR)/$(<D)
	$(CC) $(CPFFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

.PHONY: flash-target
flash-target: $(BINDIR)/$(TARGET).hex
	$(CURDIR)/tools/flash.sh ${JLINK_DEVICE} $^

.PHONY: debug-target
debug-target: flash-target
	$(CURDIR)/tools/debug.sh ${JLINK_DEVICE} $(BINDIR)/$(TARGET).elf

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)/$(BOARD)/$(CONFIG)

.PHONY: distclean
distclean:
	rm -rf $(BUILDDIR)

-include $(DEPS)
