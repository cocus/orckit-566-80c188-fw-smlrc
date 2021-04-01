LOAD_OFFSET = 0
#0x200

OUTFILE = test

TARGET_SRCS = testx.c 80c186_stuff.c or566_base.c

TARGET_CFLAGS ?= -DTEST_RODATA -DLOAD_OFFSET=$(LOAD_OFFSET)

SMLRCC ?= smlrcc
CC ?= gcc

# Locate SMLRCC path
WHERE_IS_SMLRCC ?= $(shell dirname `which $(SMLRCC)`)
# Now, grab the vXXXX directory (to assume where the include dir is)
SMLRC_V = $(shell basename $(WHERE_IS_SMLRCC)/v*)
SMLRC_INCLUDE_DIR = $(WHERE_IS_SMLRCC)/$(SMLRC_V)/include
# include the smlrc's include directory
TARGET_CFLAGS += -I$(SMLRC_INCLUDE_DIR) -SI$(SMLRC_INCLUDE_DIR)

# === Directories
BUILD_DIR = build
BUILD_OBJ_DIR = $(BUILD_DIR)/obj
BUILD_BIN_DIR = $(BUILD_DIR)/bin
BUILD_HOST_BIN_DIR = $(BUILD_DIR)/host-bin
DIRS = $(BUILD_DIR) $(BUILD_OBJ_DIR) $(BUILD_BIN_DIR) $(BUILD_HOST_BIN_DIR)

# === Target object files (+ c0d.o startup)
TARGET_OBJS = $(addprefix $(BUILD_OBJ_DIR)/,c0d.o $(TARGET_SRCS:.c=.o))
# === Target output files
OUTFILE_EXE = $(BUILD_OBJ_DIR)/$(OUTFILE).exe
OUTFILE_BIN = $(BUILD_OBJ_DIR)/$(OUTFILE).bin
OUTFILE_HEX = $(BUILD_BIN_DIR)/$(OUTFILE).hex

# === Strip app
STRIP_SRCS = stripper.c
STRIP = $(BUILD_HOST_BIN_DIR)/stripper

.PHONY: all

all: $(DIRS) $(OUTFILE_HEX)

$(OUTFILE_HEX): $(OUTFILE_BIN)
	@echo "== BIN->HEX $< -> $(@)"
	@srec_cat $< -Binary -offset $(LOAD_OFFSET) -Output $@ -Intel
$(OUTFILE_BIN): $(OUTFILE_EXE) | $(STRIP) 
	@echo "== Stripping EXE into BIN $(@)"
	@$(STRIP) $< $@
$(OUTFILE_EXE): $(TARGET_OBJS)
	@echo "== Linking $(@) from $^"
	@$(SMLRCC) $(TARGET_CFLAGS) -small $^ -o $(@)
$(BUILD_OBJ_DIR)/%.o: %.asm
	@echo "== Assembling $< -> $(@)"
	@$(SMLRCC) $(TARGET_CFLAGS) -small -c $< -o $(@)
$(BUILD_OBJ_DIR)/%.o: %.c
	@echo "== Compiling $< -> $(@)"
	@$(SMLRCC) $(TARGET_CFLAGS) -small -c $< -o $(@)
$(STRIP): $(STRIP_SRCS)
	@echo "== HOST Compiling $< -> $(@)"
	@$(CC) $(CFLAGS) $< -o $(@)
$(DIRS):
	@echo "== Creating dir $(@)"
	@mkdir -p $(@)

.PHONY: clean
clean:
	@rm -f $(TARGET_OBJS)
	@rm -rf $(DIRS)
