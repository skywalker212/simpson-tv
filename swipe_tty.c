#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <time.h>

#define DEVICE_PATH "/dev/input/event0"  // Adjust according to your device specifics
#define DOUBLE_TAP_THRESHOLD 300000000   // Double tap threshold in nanoseconds (300 ms)

int current_tty = 1;  // Starting TTY

void change_tty(int tty) {
    char command[50];
    sprintf(command, "chvt %d", tty);
    system(command);
    current_tty = tty;
}

int main() {
    int fd, rb;
    struct input_event ev;
    struct timespec last_tap_time = {0, 0};
    struct timespec now;

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

        if (ev.type == EV_KEY && ev.code == BTN_TOUCH && ev.value == 1) { // Touch press event
            clock_gettime(CLOCK_MONOTONIC, &now);

            if ((now.tv_sec - last_tap_time.tv_sec) * 1000000000L + (now.tv_nsec - last_tap_time.tv_nsec) < DOUBLE_TAP_THRESHOLD) {
                // Double tap detected
                int prev_tty = current_tty > 1 ? current_tty - 1 : 5; // Wrap around to TTY 5 if currently at TTY 1
                change_tty(prev_tty);
            } else {
                // Single tap detected
                int next_tty = current_tty < 5 ? current_tty + 1 : 1; // Wrap around to TTY 1 if currently at TTY 5
                change_tty(next_tty);
            }

            last_tap_time = now; // Update last tap time
        }
    }

    close(fd);
    return 0;
}
