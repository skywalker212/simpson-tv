# simpson-tv

This project transforms your Raspberry Pi into a dedicated video playback device, ideal for display installations like a DIY Simpsons TV. It features two primary components: `video_player` and `button_handler`. These components interact to create a user-friendly media player with simple physical button-based controls. This project is inspired by [Simpsons TV](https://withrow.io/simpsons-tv-build-guide-waveshare)

## `encode.py`

The `encode.py` script is a utility designed to encode video files into a format optimized for efficient playback on the Raspberry Pi. It utilizes the `ffmpeg` tool to transcode videos into a format compatible with the hardware acceleration capabilities of the Raspberry Pi's GPU.

### Functionality

1. **Video Detection**: The script identifies video files within a specified directory by checking file extensions (e.g., `.mp4`, `.mkv`, `.mov`, `.avi`).

2. **Video Encoding**: For each detected video file, the script transcodes it into the MP4 format using the H.264 codec. This format is preferred for its compatibility with hardware-accelerated decoding on the Raspberry Pi.

3. **Optimization Parameters**: The encoding parameters are chosen to balance file size and playback quality. These parameters include:
   - **Resolution**: Resized to 320x240 pixels, optimized for a 2.8-inch screen.
   - **Codec**: H.264 (libx264) with the baseline profile and level 3.0.
   - **Encoding Speed**: Medium preset, balancing speed and file size.
   - **Compression**: Constant Rate Factor (CRF) of 26 for moderately high compression.
   - **Pixel Format**: YUV420P for compatibility with hardware decoding.

4. **Concurrency**: The script uses a thread pool to parallelize the encoding process, improving efficiency by leveraging multiple CPU cores.

Encoded video files will be saved in a subdirectory named `encoded` within the specified directory.

## `video_player`

### Description

The `video_player` is a custom C program designed to efficiently manage and play a list of video files on the Raspberry Pi. It uses `omxplayer`, a command-line media player optimized for the Raspberry Pi's GPU, ensuring smooth video playback even on less powerful models like the Raspberry Pi Zero.

### Features

- **Hardware Accelerated Playback**: Utilizes the GPU for decoding video, which minimizes CPU usage and ensures smooth playback.
- **Automatic Video Looping**: Automatically loops through a playlist of videos stored in a specified directory.
- **Shuffle Playback**: Randomizes the order of videos to provide a varied viewing experience each cycle.
- **Signal Handling**: Listens for signals to pause/resume playback and skip to the next video, allowing external control through physical buttons or other means.

### Usage

The `video_player` is executed at startup and continuously loops through all MP4 files located in a specified directory. It can be controlled externally via UNIX signals to pause/resume and change videos.


## `button_handler`

### Description

The `button_handler` is a C program designed to interface with physical buttons connected to the GPIO pins of the Raspberry Pi. It provides physical controls for the video playback managed by `video_player`, enhancing user interaction without the need for a graphical interface.

### Features

- **Multi-Function Button Input**: Differentiates between single, double, and long button presses to provide multiple controls:
  - **Single Press**: Pauses or resumes the video playback.
  - **Double Press**: Skips to the next video in the playlist.
  - **Long Press**: Can be configured to perform actions like shutting down the Raspberry Pi or resetting the playlist.
- **Debounce Logic**: Implements software debouncing to ensure reliable button press detection.
- **Signal Emission**: Sends specific UNIX signals to `video_player` based on button inputs to control video playback.

### Usage

The `button_handler` monitors configured GPIO pins for button presses and sends commands to `video_player` based on user interactions. It must be started at boot to listen for button presses continuously.

## Optimizing boot time

### 1. **Use Raspberry Pi OS Lite**
The Lite version of Raspberry Pi OS, doesn't include many of the desktop environment components, making it inherently faster to boot up than the full version. You've likely already chosen this, but it's important to reinforce its benefit for boot time optimization.

### 2. **Disable Unnecessary Services**
Raspberry Pi OS starts several services at boot that may not be necessary. Disabling these can speed up booting:

- **Disable HDMI** if your project doesn't require video output to a monitor:
  ```bash
  /usr/bin/tvservice -o
  ```
  Add the above line to `/etc/rc.local` before `exit 0` to disable HDMI output at boot.

### 3. **Optimize `/boot/config.txt`**
Adjust settings in `/boot/config.txt` to skip some checks and speed up the boot process:
- **Disable the Rainbow Screen**: Add `disable_splash=1`.
- **Reduce boot delay**: Set `boot_delay=0` to skip the delay before boot code execution.

### 4. **Limit Kernel and Service Logging**
Verbose logging can slow down the boot process:
- Configure `rsyslog` or `systemd-journald` to limit what is logged.
- Edit `/boot/cmdline.txt` and adjust the `console=` settings to reduce the amount of logging to the console during boot.

### 5. **Asynchronous Service Starting**
Make sure services start asynchronously to avoid waiting for one to finish before starting another. This is generally handled by systemd but check that dependencies are managed correctly so that non-critical services do not block the boot sequence.

### 6. **Profile the Boot Process**
Use `systemd-analyze` to find out what's taking up the most time during boot:
```bash
systemd-analyze
systemd-analyze blame
systemd-analyze critical-chain
```
This tool shows you a breakdown of how long each service takes to start. Focus on optimizing or disabling the slowest services.

## Steps to Configure a Read-Only Filesystem

### 1. **Backup Your Data**
Before making any changes, ensure you have a backup of your SD card or important data.

### 2. **Prepare Your Raspberry Pi**
- Update your system to ensure all packages are up to date:
  ```bash
  sudo apt-get update
  sudo apt-get upgrade
  ```

### 3. **Minimize Writing to the Disk**
- Move log files to a temporary filesystem (tmpfs):
  ```bash
  sudo nano /etc/fstab
  ```
  Add these lines:
  ```
  tmpfs        /tmp            tmpfs   defaults,noatime,nosuid,size=100m        0       0
  tmpfs        /var/log        tmpfs   nosuid,nodev                             0       0
  tmpfs        /var/tmp        tmpfs   nosuid,nodev                             0       0
  ```
  Save and close the file.

### 4. **Adjust Boot Configuration**
- Edit the `cmdline.txt` to mount the root filesystem as read-only:
  ```bash
  sudo nano /boot/cmdline.txt
  ```
  Find the root entry and modify it from `root=/dev/mmcblk0p2` to:
  ```
  root=/dev/mmcblk0p2 ro
  ```
  Additionally, add `fastboot noswap ro` at the end of the line.

### 5. **Make System Changes for Read-Only Compatibility**
- Make DHCP client changes:
  ```bash
  sudo rm /etc/systemd/system/dhcpcd5
  sudo ln -s /lib/systemd/system/dhcpcd.service /etc/systemd/system/dhcpcd.service.dhcpcd5
  ```
- Edit `dhcpcd` configuration to use a static IP or ensure it doesn't need to write:
  ```bash
  sudo nano /etc/dhcpcd.conf
  ```
  This is optional and depends on whether you are using a static IP.

- Adjust `systemd` to handle read-only root:
  ```bash
  sudo systemctl mask systemd-random-seed.service
  sudo ln -s /dev/null /etc/tmpfiles.d/dhcpcd.conf
  sudo ln -s /dev/null /etc/systemd/system/sysinit.target.wants/systemd-random-seed.service
  ```

### 6. **Remount Root Filesystem as Read-Write When Needed**
To make changes after this setup, you'll need to remount the filesystem as read-write:
```bash
sudo mount -o remount,rw /
```
When done, remount it as read-only:
```bash
sudo mount -o remount,ro /
```

Check if everything boots up correctly and your applications function as expected in a read-only environment.

### Disable Networking and Bluetooth on Raspberry Pi

#### 1. **Disable Networking**
To completely disable networking on your Raspberry Pi, you can unload and blacklist the kernel modules responsible for network interfaces. Here's how to do it:

1. **Open the cmdline.txt file** to add networking disable commands:
    ```bash
    sudo nano /boot/cmdline.txt
    ```
    Add `ip=off` to the end of the line to disable DHCP and networking services.

2. **Blacklist Network Kernel Modules**:
    Create a new blacklist file in the `/etc/modprobe.d/` directory:
    ```bash
    sudo nano /etc/modprobe.d/raspi-blacklist.conf
    ```
    Add the following lines to disable Ethernet and Wi-Fi modules:
    ```
    blacklist brcmfmac
    blacklist brcmutil
    blacklist smsc95xx
    blacklist lan78xx
    ```
    This disables the drivers for the onboard WiFi and Ethernet (depending on your model of Raspberry Pi).

#### 2. **Disable Bluetooth**
To disable Bluetooth, you need to blacklist its kernel modules and disable its system services.

1. **Add to the Blacklist File**:
    Open the previously created blacklist file:
    ```bash
    sudo nano /etc/modprobe.d/raspi-blacklist.conf
    ```
    Add the following lines:
    ```
    blacklist btbcm
    blacklist hci_uart
    ```
2. **Disable Related Services**:
    Stop and disable Bluetooth services:
    ```bash
    sudo systemctl disable bluetooth.service
    sudo systemctl disable hciuart.service
    ```

#### 3. **Edit the config.txt File**
Disable the hardware interfaces directly from the bootloader configuration:

1. **Edit config.txt**:
    ```bash
    sudo nano /boot/config.txt
    ```
    Add these lines to the end of the file:
    ```
    dtoverlay=disable-wifi
    dtoverlay=disable-bt
    ```
    This will completely disable the onboard WiFi and Bluetooth hardware interfaces.

#### 4. **Optimize Boot Configurations**
Ensure that no unnecessary services are started at boot:

- **Modify systemd services** to prevent networking services from starting:
    ```bash
    sudo systemctl disable dhcpcd.service
    sudo systemctl mask networking.service
    ```

#### 5. **Reboot**
After making these changes, reboot your Raspberry Pi to ensure all changes take effect and the modules are not loaded:
```bash
sudo reboot
```

### Testing and Validation
- After rebooting, check that the network interfaces are indeed disabled:
  ```bash
  ifconfig -a
  ```
  This command should not show any active network interfaces except for `lo` (local loopback).

- Check Bluetooth status:
  ```bash
  hciconfig
  ```
  There should be no Bluetooth devices listed.

### Installation of Systemd Services

To use these systemd service files:

1. **Place the service files** in `/etc/systemd/system/` directory.
   ```bash
   sudo nano /etc/systemd/system/tvplayer.service
   sudo nano /etc/systemd/system/tvbutton.service
   ```
   Copy and paste the content for each service file respectively and save them.

2. **Reload the systemd manager configuration** to recognize changes to service files.
   ```bash
   sudo systemctl daemon-reload
   ```

3. **Enable the services** to start at boot.
   ```bash
   sudo systemctl enable tvplayer.service
   sudo systemctl enable tvbutton.service
   ```

4. **Start the services**.
   ```bash
   sudo systemctl start tvplayer.service
   sudo systemctl start tvbutton.service
   ```

5. **Check the status** of the services to ensure they are running correctly.
   ```bash
   sudo systemctl status tvplayer.service
   sudo systemctl status tvbutton.service
   ```