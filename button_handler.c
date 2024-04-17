#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>

#define BUTTON_PIN 26 // according to wiringPi setup
#define SCREEN_PIN 18    // GPIO pin for the screen power
#define PAUSE_SIGNAL SIGUSR1
#define NEXT_SIGNAL SIGUSR2
int VIDEO_PLAYER_PID = -1;
int SCREEN_ON = 1;

void turnOnScreen() {
    digitalWrite(SCREEN_PIN, HIGH);
    printf("Screen turned ON\n");
}

void turnOffScreen() {
    digitalWrite(SCREEN_PIN, LOW);
    printf("Screen turned OFF\n");
}

void singlePressAction() {
    kill(VIDEO_PLAYER_PID, PAUSE_SIGNAL);
}

void doublePressAction() {
    kill(VIDEO_PLAYER_PID, NEXT_SIGNAL);
}

void triplePressAction() {
    printf("Triple press action triggered.\n");
}

void longPressAction() {
    if (SCREEN_ON) turnOffScreen();
    else turnOnScreen();
    SCREEN_ON != SCREEN_ON;
}

// Helper function to get current time in milliseconds
long long currentTimeMillis() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}

long long lastPressTime = 0;
int pressCount = 0;
long long pressStartTime = 0;

void buttonPressInterrupt(void) {
    long long currentTime = currentTimeMillis();
    if (currentTime - lastPressTime > 200) { // Basic debounce control
        pressStartTime = currentTime;
    }
    lastPressTime = currentTime;
}

void buttonReleaseInterrupt(void) {
    long long currentTime = currentTimeMillis();
    long pressDuration = currentTime - pressStartTime;

    if (pressDuration > 1500) { // Check for long press
        longPressAction();
    } else {
        pressCount++;
        if (pressCount == 1) {
            singlePressAction();
        } else if (pressCount == 2) {
            doublePressAction();
        } else if (pressCount == 3) {
            triplePressAction();
            pressCount = 0; // Reset after triple press
        }
    }
}

int main(void) {
    FILE *fp;

    fp = fopen("/tmp/video_player_pid", "r");
    if (fp == NULL) {
        perror("Failed to open pid file");
        return -1;
    }
    fscanf(fp, "%d", &VIDEO_PLAYER_PID);
    fclose(fp);

    wiringPiSetupGpio(); // Use Broadcom pin numbers
    pinMode(BUTTON_PIN, INPUT);
    pullUpDnControl(BUTTON_PIN, PUD_UP);
    pinMode(SCREEN_PIN, OUTPUT);

    // Setting up the interrupt for button press
    if (wiringPiISR(BUTTON_PIN, INT_EDGE_FALLING, &buttonPressInterrupt) < 0) {
        fprintf(stderr, "Unable to setup ISR for falling edge: %s\n", strerror(errno));
        return 1;
    }

    // Setting up the interrupt for button release
    if (wiringPiISR(BUTTON_PIN, INT_EDGE_RISING, &buttonReleaseInterrupt) < 0) {
        fprintf(stderr, "Unable to setup ISR for rising edge: %s\n", strerror(errno));
        return 1;
    }

    // Main loop does nothing, all handling is done in interrupts
    while (1) {
        sleep(1); // Sleep to reduce CPU usage, main loop does nothing
    }

    return 0;
}
