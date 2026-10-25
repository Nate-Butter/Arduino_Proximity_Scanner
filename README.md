# Arduino Proximity Scanner

An Arduino-based embedded system that continuously scans its surroundings, measures object distance, and provides visual and audible warnings based on proximity.

## Project Overview

This project was built to practice embedded systems programming and integrating multiple hardware components into one system.

A servo motor continuously sweeps an HC-SR04 ultrasonic sensor across an area. The sensor measures object distance, while a 16x2 LCD displays the current measurement. An RGB LED and passive buzzer indicate different proximity levels.

## Current Features - Version 1

- Continuous servo scanning from 20° to 160°
- Real-time ultrasonic distance measurement
- Distance displayed in feet and inches on a 16x2 LCD
- Green indicator for safe distances
- Yellow indicator for the warning zone
- Red danger state for close objects
- Audible warnings with different beep rates based on distance
- Non-blocking timing using `millis()`
- Modular C++ functions for each subsystem

## Hardware

- Arduino Uno
- HC-SR04 ultrasonic sensor
- Servo motor
- 16x2 LCD display
- Common-cathode RGB LED
- Passive buzzer
- Potentiometer
- Breadboard
- Resistors
- Jumper wires
- External breadboard power supply

## How It Works

The servo continuously sweeps the ultrasonic sensor between 20° and 160°.

The ultrasonic sensor sends a pulse and measures how long it takes for the echo to return. The Arduino uses this measurement to calculate the object's distance.

**Safe:** Green LED, buzzer off

**Warning:** Yellow LED, slow audible warning

**Danger:** Red LED, faster and higher-pitched audible warning

The measured distance is simultaneously displayed on the LCD in feet and inches.

## Software Structure

- `updateServo()` - controls scanning motion
- `measureDistance()` - reads the ultrasonic sensor
- `updateLCD()` - displays the current distance
- `updateWarningSystem()` - determines the warning state
- `updateBuzzer()` - controls non-blocking audible alerts
- `setColor()` - controls the RGB LED

Using `millis()` allows multiple parts of the system to operate without relying heavily on blocking delays.

## Known Hardware Issue

The red channel of the RGB LED used during Version 1 testing is currently nonfunctional due to a hardware issue with the LED itself. The warning-state logic is implemented in software, but the LED will be replaced.

## Project Build

<img src="IMG_5657.jpeg" width="500">

### Hardware Setup

<p>
  <img src="IMG_5655.jpeg" width="350">
  <img src="IMG_5654.jpeg" width="350">
</p>

## Demo

Version 1 demonstrates continuous area scanning, real-time distance measurement, and automatic visual and audible warning responses when an object enters the warning or danger zones.

[▶ Watch the Version 1 Demo](https://youtube.com/shorts/jyxK6KUi9S4?si=MJRMJU0ft94h1kxm)

## Experimental Object Tracking (Version 2 Prototype)

After completing Version 1, I experimented with adding object tracking using the **same Arduino Uno, HC-SR04 ultrasonic sensor, and servo setup**.

I implemented a state-machine approach with three operating modes:

- **SCANNING:** Sweep the sensor to detect nearby objects.
- **TRACKING:** Attempt to follow a detected object by adjusting the servo position.
- **SEARCHING:** Attempt to reacquire an object after losing it.

The prototype achieved basic tracking, but its responsiveness was below my goal. In testing, the sensor sometimes struggled to follow an object moving across its field of view. I suspected the single ultrasonic sensor and its need to rotate between measurements contributed to the delays, but I did not conclusively isolate the cause from servo response or algorithm behavior.

This experiment showed me how hardware sensing limitations can affect the performance of a software control algorithm. If I revisit the project, I would test alternative sensing arrangements and compare tracking response times.

**Documentation note:** This was an experimental extension, not part of the published Version 1 code or demo. I did not preserve a separate tracking implementation or demonstration, and the prototype has since been disassembled.

## Project Status

**Version 1:** Working and documented in this repository.

**Version 2:** Basic object tracking was prototyped and tested, but did not meet the intended responsiveness. The prototype is no longer assembled.
