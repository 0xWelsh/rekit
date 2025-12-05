CC = gcc
CFLAGS = -Wall -O2
LIBS = -lcapstone

BIN_DIR = bin
DBI_DIR = dbi
PARSERS_DIR = parsers
TOOLS_DIR = tools
ANALYSIS_DIR = analysis

TARGETS = $(BIN_DIR)/dbi-framework \
          $(BIN_DIR)/dbi-advanced \
          $(BIN_DIR)/syscall-tracer \
          $(BIN_DIR)/pe-parser \
          $(BIN_DIR)/elf-parser \
          $(BIN_DIR)/memdump \
          $(BIN_DIR)/strings \
          $(BIN_DIR)/anti-debug-detect

all: $(BIN_DIR) $(TARGETS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/dbi-framework: $(DBI_DIR)/dbi-framework.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

$(BIN_DIR)/dbi-advanced: $(DBI_DIR)/dbi-advanced.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

$(BIN_DIR)/syscall-tracer: $(DBI_DIR)/syscall-tracer.c
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/pe-parser: $(PARSERS_DIR)/pe-parser.c
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/elf-parser: $(PARSERS_DIR)/elf-parser.c
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/memdump: $(TOOLS_DIR)/memdump.c
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/strings: $(ANALYSIS_DIR)/strings.c
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/anti-debug-detect: $(ANALYSIS_DIR)/anti_debug_detect.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BIN_DIR)

install:
	cp $(BIN_DIR)/* /usr/local/bin/

.PHONY: all clean install

$(BIN_DIR)/rekit-gui: gui/rekit-gui.c
	$(CC) $(CFLAGS) -o $@ $< `pkg-config --cflags --libs gtk+-3.0`

gui: $(BIN_DIR) $(BIN_DIR)/rekit-gui

.PHONY: gui
