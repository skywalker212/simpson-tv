CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lwiringPi

# Define the source files and target binaries
VIDEO_PLAYER_SRC = video_player.c
VIDEO_PLAYER_BIN = video_player

BUTTON_HANDLER_SRC = button_handler.c
BUTTON_HANDLER_BIN = button_handler

all: $(VIDEO_PLAYER_BIN) $(BUTTON_HANDLER_BIN)

$(VIDEO_PLAYER_BIN): $(VIDEO_PLAYER_SRC)
	$(CC) $(CFLAGS) $(VIDEO_PLAYER_SRC) -o $(VIDEO_PLAYER_BIN)

$(BUTTON_HANDLER_BIN): $(BUTTON_HANDLER_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(BUTTON_HANDLER_SRC) -o $(BUTTON_HANDLER_BIN)

clean:
	rm -f $(VIDEO_PLAYER_BIN) $(BUTTON_HANDLER_BIN)
