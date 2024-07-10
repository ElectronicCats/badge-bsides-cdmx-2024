# BSides badge developer guide

This document is intended to help developers get started with the BSides badge firmware. The badge is based on the ESP32 microcontroller and is programmed using the ESP-IDF framework.

## Getting started

1. Install the ESP-IDF framework by following the instructions at https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html

2. Clone the BSides badge firmware repository:

Using HTTPS:
```
git clone https://github.com/ElectronicCats/Minino.git
```

Using SSH:
```
git clone git@github.com:ElectronicCats/Minino.git
```

3. Change to the firmware directory:
```
cd badge-bsides-cdmx-2024/firmware/badge
```

4. Connect your badge to your computer using a USB cable and check the port it is connected to.

> On Windows, you can check it in the Device Manager, on Linux you can check it with the `ls /dev/ttyUSB*` or `ls /dev/ttyACM*` command, and on macOS you can check it with the `ls /dev/cu.usb*` command.

6. Set the target to ESP32-C6:

```bash
idf.py set-target esp32c6
```

6. Build the firmware:

```bash
idf.py build
```

7. Flash the firmware:

```bash
idf.py -p $PORT flash
```

> Replace `$PORT` with the port your badge is connected to.

8. Monitor the output:

```bash
idf.py -p $PORT monitor
```

> You can do this steps at once by running `make all`.

## Using badge_connect_src library

The project uses the [badge_connect_bin](https://github.com/ElectronicCats/badge_connect_bin) library to interact with badges from different events. It is a pre-compiled version, so you can't modify it. If you need to debug, modify or add new features, you can use the source version.

To use the source version, follow these steps:

1. Clone the badge_connect_src repository (Not inside this repository, of course):

Using HTTPS:

```bash
git clone https://github.com/ElectronicCats/badge_connect_src.git
```

Using SSH:

```bash
git clone git@github.com:ElectronicCats/badge_connect_src.git
```

2. Get the path to the badge_connect_src directory:

```bash
cd badge_connect_src/
pwd
```

3. Set the path to an environment variable:

```bash
export BADGE_CONNECT_SRC_PATH=/path/to/badge_connect_src
```

That's it! Now you are using the source version. To verify it, go to `firmware/badge` in this project and run `idf.py menuconfig`. It should show you an interface where you can modify some options, if you see `Badge Connect  --->` you are using the source version.

To go back to the binary version, just unset the environment variable:

```bash
unset BADGE_CONNECT_SRC_PATH
```
