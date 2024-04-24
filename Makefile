CC = gcc
CFLAGS = -Wall -g
INSTALL_PATH = /usr/local/bin

# Define source files and target binaries
SOURCES = video_player.c button_handler.c swipe_tty.c
OBJECTS = $(SOURCES:.c=.o)
TARGETS = $(SOURCES:.c=)

all: $(TARGETS)

video_player: video_player.o
	$(CC) $(CFLAGS) -o $@ $^

button_handler: button_handler.o
	$(CC) $(CFLAGS) -o $@ $^ -lwiringPi

swipe_tty: swipe_tty.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install:
	@echo "Installing executables to $(INSTALL_PATH)"
	@$(foreach exec,$(TARGETS),cp $(exec) $(INSTALL_PATH)/$(exec);)
	@echo "Installation complete."

clean:
	rm -f $(TARGETS) $(OBJECTS)

.PHONY: all install clean
