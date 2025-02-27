# Adaptive Feedback Canceller

This project implements an adaptive feedback canceller for audio processing using a combination of notch and LMS filters. The system is designed to run on an embedded platform that support Arduino framework and includes a graphical user interface (GUI) for control and monitoring.

## Features

- Adaptive feedback cancellation using notch and LMS filters
- Real-time audio processing
- Serial communication for control and monitoring
- Graphical user interface for easy control and visualization

## Hardware Requirements

- Arduino board (e.g., Teensy)
- Audio input and output hardware (e.g., I2S microphone and speaker)
- Optional: Button and potentiometer for manual control

## Software Requirements

- Arduino IDE
- Python 3.x
- Required Python packages: `tkinter`, `matplotlib`, `pyserial`, `numpy`

## Installation

1. **Arduino Setup:**
   - Install the Arduino IDE from [Arduino website](https://www.arduino.cc/en/software).
   - Install the required libraries in the Arduino IDE:
     - `Audio`
     - `Wire`
     - `SPI`
     - `SD`
   - Open the `src/main.cpp` file in the Arduino IDE and upload it to your Arduino board.

2. **Python Setup:**
   - Install Python 3.x from [Python website](https://www.python.org/downloads/).
   - Install the required Python packages using pip:
     ```sh
     pip install matplotlib pyserial numpy
     ```
   - You can also install the required packages using the provided `requirements.txt` file:
     ```sh
     pip install -r requirements.txt
     ```

## Usage

1. **Connect Hardware:**
   - Connect the audio input and output hardware to the Arduino board.
   - Optionally, connect a button and potentiometer for manual control.

2. **Run the GUI:**
   - Open a terminal and navigate to the `scripts` directory.
   - Run the `teensy_monitor.py` script:
     ```sh
     python teensy_monitor.py
     ```

3. **Control and Monitor:**
   - Use the GUI to connect to the Arduino board via the specified serial port.
   - Adjust the gain, enable/disable LMS and notch filters, and monitor the status and frequency analysis in real-time.

## File Structure

- `src/`: Contains the Arduino source code.
  - `main.cpp`: Main Arduino program.
  - `AdaptiveFeedbackCanceller.h` and `AdaptiveFeedbackCanceller.cpp`: Adaptive feedback canceller implementation.
  - `NotchLMSFilter.h` and `NotchLMSFilter.cpp`: Notch and LMS filter implementation.
  - `LMSFilter.h` and `LMSFilter.cpp`: LMS filter implementation.
  - `NotchFilter.h` and `NotchFilter.cpp`: Notch filter implementation.
- `scripts/`: Contains the Python scripts for the GUI.
  - `teensy_monitor.py`: Main GUI script.
- `README.md`: This file.