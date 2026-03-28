# Compiler
CC      = cc
CFLAGS  = -Wall -Wextra -O0 -g

CFLAGS += -I. -Ihandlers

# Target
TARGET  = netagentd

# Directories
HANDLERS_DIR = handlers

HANDLER_SRCS!= ls ${HANDLERS_DIR}/*.c

# Sources
SRCS = \
	main.c \
	server.c \
	protocol.c \
	exec_safe.c \
	auth.c \
	policy.c \
	util.c \
	handlers.c \
	$(HANDLER_SRCS)
	
# Objects
OBJS = $(SRCS:.c=.o)

# Python
VENV = .venv
PYTHON = $(VENV)/bin/python
PIP = $(VENV)/bin/pip

# Default target
all: $(TARGET)

# Build binary
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compile .c → .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build
clean:
	rm -f $(TARGET) $(OBJS)

# ---------- Python ----------

# Create virtualenv
venv:
	python3 -m venv $(VENV)

# Install library
py-install: venv
	$(PIP) install -e ../lib

# Quick test
py-test:
	$(PYTHON) -c "from netagent import NetAgentClient; print('OK')"

# Run CLI
cli:
	$(VENV)/bin/python ../cli/netagentctl list

# Full setup
setup: venv py-install py-test

.PHONY: all clean venv py-install py-test cli setup
