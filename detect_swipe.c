#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <string.h>

#define DEVICE_PATH "/dev/input/event0"  // Change according to your device specifics
#define SWIPE_THRESHOLD 100
#define MAX_TTY 5
#define MIN_TTY 1

int current_tty = 1;  // Start with TTY1

void change_tty(int tty) {
    char command[50];
    sprintf(command, "chvt %d", tty);
    system(command);
    current_tty = tty;
}

int main() {
    int fd, rb;
    struct input_event ev;
    int x_start = -1, x_end;

    fd = open(DEVICE_PATH, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Failed to open device file %s\n", DEVICE_PATH);
        return EXIT_FAILURE;
    }

    while (1) {
        rb = read(fd, &ev, sizeof(struct input_event));
        if (rb < (int) sizeof(struct input_event)) {
            perror("Read error");
            return EXIT_FAILURE;
        }

        if (ev.type == EV_ABS && ev.code == ABS_X) {
            if (x_start == -1) {
                x_start = ev.value;
            } else {
                x_end = ev.value;
                // Detect swipe direction
                if (x_start - x_end > SWIPE_THRESHOLD) {  // Left swipe
                    int next_tty = (current_tty >= MAX_TTY) ? MIN_TTY : current_tty + 1;
                    change_tty(next_tty);
                } else if (x_end - x_start > SWIPE_THRESHOLD) {  // Right swipe
                    int prev_tty = (current_tty <= MIN_TTY) ? MAX_TTY : current_tty - 1;
                    change_tty(prev_tty);
                }
                x_start = -1;  // Reset for next swipe detection
            }
        } else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
            x_start = -1;  // Reset on each report event
        }
    }

    close(fd);
    return 0;
}
