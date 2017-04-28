NAME = udapter

CFLAGS += -std=gnu99 -pedantic -Wall -Wextra -Iinclude
LDFLAGS += -levent -levent_openssl -lssl -lcrypto -levtssl -ljson-c

SRCDIR = src
OBJDIR = obj
BINDIR = bin

DIRS = $(OBJDIR) $(BINDIR)

ifeq (1,$(DEBUG))
CFLAGS += -g
else
CFLAGS += -O2
endif

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

BIN = $(BINDIR)/$(NAME)

.PHONY: all clean default debug

default: $(BIN)

debug:
	$(MAKE) DEBUG=1

$(BIN): $(SRVOBJECTS) $(OBJECTS) | $(BINDIR)
	$(CC) -o $@ $^ $(LDFLAGS)
	chmod 755 $@

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) -o $@ -c $< $(CFLAGS)

$(DIRS):
	mkdir -p $@

clean::
	rm -rf $(DIRS)
