Project for PP RAI 2024

# Core
- [x] Event loop
- [x] Wifi driver
- [x] Cam driver
- [x] Processing JPEG by edge detection algorithm 
- [x] Sending JPEG to Host
- [x] Receiving JPEG from ESP32 via server.py

### TODO
- [ ] Fix error related to Torch ai model arch: `output_shape->data[shape_size] != dim_shape(1 != 0)
Node STRIDED_SLICE (number 48f) failed to prepare with status 1`

## Installation Steps for Edge Detection AI Model on ESP

Follow the steps below to set up and run the edge detection AI model on ESP using ESP-IDF, TensorFlow, PIL, and Socket.
### 1. Install ESP-IDF

Follow the official ESP-IDF installation guide to set up the ESP-IDF development environment:

```bash

git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
```

### 2. Install Python Packages

Install the required Python packages, including TensorFlow, PIL (Pillow), and Socket:

```bash

pip install tensorflow pillow socket

```
### 3. Enable SPIRAM, AI model, setup wifi credentials

Run the following command to configure the project using menuconfig:

```bash

idf.py menuconfig

```
> Ensure WPA2 PSK for WiFi Connection

### 4. Build, Flash, and Monitor

Build the project, flash it to the ESP, and monitor the output:

```bash

idf.py build flash monitor

```
### 5. Run Server

For capturing processed jpeg:

```bash

python3 server.py tensor

```
For capturing raw jpeg:

```bash

python3 server.py jpeg

```
