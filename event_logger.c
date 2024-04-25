#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdlib.h>

#define DEVICE_PATH "/dev/input/event0"  // Adjust as necessary

int main() {
    int fd;
    struct input_event ev;

    // Open the input device
    fd = open(DEVICE_PATH, O_RDONLY);
    if (fd == -1) {
        perror("Opening device");
        return EXIT_FAILURE;
    }

    printf("Listening for input events on %s...\n", DEVICE_PATH);

    // Read input events
    while (1) {
        if (read(fd, &ev, sizeof(struct input_event)) != sizeof(struct input_event)) {
            perror("Read error");
            return EXIT_FAILURE;
        }

        // Log details about each event
        printf("Event: type %d, code %d, value %d\n", ev.type, ev.code, ev.value);

        // You can also add specific checks to confirm expected event types and codes
        if (ev.type == EV_ABS && ev.code == ABS_X) {
            printf("X coordinate: %d\n", ev.value);
        } else if (ev.type == EV_ABS && ev.code == ABS_Y) {
            printf("Y coordinate: %d\n", ev.value);
        }
    }

    close(fd);
    return 0;
}
