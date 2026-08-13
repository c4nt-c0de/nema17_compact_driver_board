# Wi-Fi-Controlled Stepper Motor Driver Board

This little project began as part of embedded programming class. I wished to design my own PCB and have it manufactured instead of choosing a project with the PCB that was available. So I tried to design a compact driver board that would screw into the back of a NEMA17 and could be controlled over Wi-Fi, as well as detect/receive IR signal  from a remote.

Before the design itself, we were required to submit a small document of what we're about to do (this file is in "study" folder). As always, there were some deviations from the initial plan due to challenges and some poor decisions.

---

## Hardware Overview

* Microcontroller: ESP32-S2-MINI-2-N4
* Stepper Driver: TMC2209-LA-T (featuring UART interface for stallGuard and configuration)
* Shaft Encoder: AS5600-ASOT
* IR Receiver: IRM-H638T-TR2-TL
* Power Supply: 
  * Motor Power: RJ45 connector (testing whether Cat5 cable can be used to bring power to the motor over longer distance than typical USB cable...)
  * Logic Power: USB-C (5V) using a TPS54331DR switching regulator.

---

## Board Renders & Layout

By far the biggest experiment in this project was attempting to have components on both sides of the PCB. At the time of designing this, JLC PCB did not offer double-sided PCBA services. If they do now, I'm sure the cost would persuade me against it. But for all the components I planned to use and still have the PCB fit on the back on a NEMA17 motor, both sides of the board had to be populated.

So I chose a standard PCBA with compoents soldered to the top side, but the board is designed in such a way, that it is then cut in half and the two halves are glued together and soldered in certain points to connect them together.

The choice of RJ45 connector for delivering power was also strange, but it was something I had on hand and by my calculations, could be used to carry 24V safely, at least at the expected currents.

Top View
![Front View](images/front_v3.png)

Bottom View
![Back View](images/back_v3.png)

Front Layout
![Front Layout](images/front_layout_v3.png)

---

## Schematics & Subsystems

### Complete System Schematic

![Complete Schematic](images/v3.svg)
<br><br>
### Power Connector

![Power Connector](images/power_connector.svg)
<br><br>
### Switching Regulator (TPS54331DR)

![Switching Regulator](images/v3-switching_regulator_TPS54331DR.svg)
<br><br>
### Stepper Driver (TMC2209)

![Stepper Driver](images/v3-stepper_driver_tmc2209.svg)
<br><br>
### Shaft Encoder (AS5600)

![Shaft Encoder](images/v3-shaft_encoder_AS5600.svg)
<br><br>
### IR Receiver (IRM-H630T)

![IR Receiver](images/v3-irrec_IRM_H630T.svg)
<br><br>
### External Crystal
![External Crystal](images/v3-extern_crystal.svg)

---

## Firmware & Logic

The firmware runs on the ESP32-S2, handling:
1. Motor Control: Managed via stepper driver libraries (e.g., [TMCStepper](https://github.com/teemuatlut/TMCStepper)).
2. Position Tracking: Sampling PWM output from the AS5600 magnetic encoder.
3. Communication: Receiving high-level motion commands and transmitting real-time telemetry (position, error flags, stall conditions) over Wi-Fi.

---

## Retrospective & Mistakes

Truth be told, this ended up being more of a failure project than a polished piece of hardware, but it turned out to be a massive learning experience. 

* **The Power Path:** Using an RJ45 connector for motor power delivery works in a pinch, but it's fundamentally sketchy and definitely not something I'd recommend for standard layouts. 
* **The Logic Voltage Drop:** A major oversight involved the clamping diode on the 5V rail right near the USB-C port. It introduced an excessive voltage drop that dragged the voltage down too far below 5V, completely stopping the TMC2209 drivers from functioning until I physically bypassed the diode and shorted the pads on the board. 

Every mistake here taught me a lot more about board bring-up and power distribution than a successful design would have.

* **The Power Path:** Using an RJ45 connector for motor power delivery[cite: 1] works in a pinch, but it's fundamentally sketchy and definitely not something I'd recommend for standard layouts[cite: 1]. 
* **The Logic Voltage Drop:** A major oversight involved the clamping diode on the 5V rail right near the USB-C port. It introduced an excessive voltage drop that dragged the voltage down too far below 5V, completely stopping the TMC2209 drivers from functioning until I physically bypassed the diode and shorted the pads on the board. 

Every mistake here taught me a lot more about board bring-up and power distribution than a successful design would have.
