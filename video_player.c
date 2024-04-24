#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#define MAX_FILES 100
#define BUFFER_SIZE 1024

char *video_files[MAX_FILES];
int video_count = 0;
volatile sig_atomic_t current_video_index = 0;
int omxplayer_pipe[2];

void shuffleVideos(char *array[], int n) {
    if (n > 1) {
        srand(time(NULL));
        for (int i = 0; i < n - 1; i++) {
            int j = i + rand() / (RAND_MAX / (n - i) + 1);
            char *temp = array[j];
            array[j] = array[i];
            array[i] = temp;
        }
    }
}

void getVideos(const char *directory) {
    DIR *dir;
    struct dirent *entry;
    if ((dir = opendir(directory)) == NULL) {
        perror("opendir() failed");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".mp4")) {
            char *full_path = malloc(BUFFER_SIZE);
            snprintf(full_path, BUFFER_SIZE, "%s/%s", directory, entry->d_name);
            video_files[video_count++] = full_path;
            if (video_count >= MAX_FILES) break;
        }
    }
    closedir(dir);
}

void signal_pause(int sig) {
    write(omxplayer_pipe[1], " ", 1);  // Send pause toggle command to omxplayer
}

void signal_next_video(int sig) {
    write(omxplayer_pipe[1], "q", 1);  // Send quit command to omxplayer to move to next video
}

void setup_signals() {
    struct sigaction sa_pause, sa_next;
    memset(&sa_pause, 0, sizeof(sa_pause));
    sa_pause.sa_handler = signal_pause;
    sigaction(SIGUSR1, &sa_pause, NULL);

    memset(&sa_next, 0, sizeof(sa_next));
    sa_next.sa_handler = signal_next_video;
    sigaction(SIGUSR2, &sa_next, NULL);
}

void playVideo(const char *video_path) {
    if (pipe(omxplayer_pipe) != 0) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    const char *args[] = {"omxplayer", "--no-osd", "--aspect-mode", "fill", video_path, NULL};
    pid_t pid = fork();

    if (pid == 0) {
        close(omxplayer_pipe[1]);  // Close the write end of the pipe in the child process
        dup2(omxplayer_pipe[0], STDIN_FILENO);  // Redirect stdin to read from the pipe
        execvp("omxplayer", (char *const *)args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        close(omxplayer_pipe[0]);  // Close the read end of the pipe in the parent
        wait(NULL);  // Wait for the child process to finish
    } else {
        perror("fork failed");
    }
}

void playVideos() {
    if (video_count == 0) {
        fprintf(stderr, "No videos found.\n");
        return;
    }

    shuffleVideos(video_files, video_count);
    for (int i = 0; i < video_count; i++) {
        playVideo(video_files[i]);
    }
}

int main() {
    setup_signals();
    // write video player pid for the button process to read
    FILE *fp = fopen("/tmp/video_player_pid", "w");
    if (fp == NULL) {
        perror("Failed to open pid file");
        return EXIT_FAILURE;
    }
    fprintf(fp, "%d", getpid());
    fclose(fp);
    const char *directory = "/home/homer/simpson-tv/videos";
    getVideos(directory);
    playVideos();
    return 0;
}
