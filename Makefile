# Compiler
CC := gcc

#Compiler flags
CFLAGS := -Wall -Wextra

# Source code file
SOURCE := journalnet.c

# Executable file
TARGET := journalnet

INSTALL_DIR = /usr/local/bin

# Default target
all: $(TARGET)

# Build the target
$(TARGET): $(SOURCE)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

install:
	@if [ $$(id -u) -ne 0 ]; then \
		echo "You must run 'make install' as root (UID 0) to install the program."; \
		exit 1; \
	fi

	install -m 755 $(TARGET) $(INSTALL_DIR)

# Clean generated files
clean:
	rm -f $(TARGET)
  
