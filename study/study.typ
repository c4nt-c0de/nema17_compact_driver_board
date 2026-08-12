= Wi-Fi-Controlled Stepper Motor Driver Board \ for NEMA17 Motors

author: Jakub Benetin
== 1. Purpose
The proposed PCB is supposed to be a compact solution 
for driving NEMA17-style motors over #box[Wi-Fi] with shaft position feedback.
Some solutions for this already exist on the market, such as: \
#show link: underline
https://github.com/joshr120/PD-Stepper \
My design includes a few extra sensors and a power connector more suitable
for power delivery over longer distance than #box[USB-C] cable.
== 2. Hardware Overview

=== Microcontroller
ESP32-S2-MINI-2-N4  
#link("https://documentation.espressif.com/esp32-s2-mini-2_esp32-s2-mini-2u_datasheet_en.pdf")[(datasheet)]

=== Peripherals
- IR receiver: IRM-H638T-TR2-TL #link("https://www.everlighteurope.com/custom/files/datasheets/DMO-0000008.pdf")[(datasheet)]
- Shaft encoder: AS5600-ASOT #link("https://www.lcsc.com/datasheet/C499458.pdf")[(datasheet)]
- Stepper driver: TMC2209-LA-T #link("https://www.analog.com/media/en/technical-documentation/data-sheets/tmc2209_datasheet_rev1.09.pdf")[(datasheet)]
- 32.768 kHz external crystal #link("https://www.lcsc.com/datasheet/C32346.pdf")[(datasheet)]

=== Power
- Switching regulator: TPS54331DR #link("https://www.lcsc.com/datasheet/C9865.pdf")[(datasheet)]

- Motor power input: RJ45 connector, #box[5–28 V DC] (motors are expected to work from 12-24V)
- ESP32 power input: USB-C (5V) should power up the ESP32
\
\
\
#figure(
    image("schematics/power_connector.svg", width: 15cm),
    caption: [Unconventional RJ45 connector used for power \ 
    (it works, but I wouldnt recommend it...)]
)

#pagebreak()
== 3. Pin Assignments
#set table(
  stroke: 0.5pt + black,  // border around cells
  align: center,
  inset: (y:10pt)
)
#show table.cell.where(y: 0): strong

#table(
  columns: (50pt, 1fr, 2fr, 3fr),
  table.header(
    [],
    [MCU GPIO],
    [Function],
    [Notes],
  ),

  // TMC2209 section
  table.cell(
    rowspan: 8,
    align: center,
    fill: rgb("#5947ff")
  )[
    #pad(top: 2.8cm)[
      #rotate(-90deg)[TMC2209]
    ]
  ],
  [1], [DIR], [-], 
  [2], [STEP], [-], 
  [3], [INDEX], [increments each full motor step (not used)], 
  [4], [DIAG], [configure for interrupts on stall], 
  [5], [ENN], [-], 
  [6], [STDBY], [low power mode (coils unpowered)], 
  [17], [RX1], [listens for flags from TMC2209], 
  [18], [TX1], [configures TMC2209],

  // Touch pins
  table.cell(
    rowspan: 2,
    align: center,
    fill: rgb("#13c2c2")
  )[
    #rotate(-90deg)[Touch pins]
  ],
  [7], [TOUCH1], [optional input for motor control], 
  [8], [TOUCH2], [same as above],

  // External crystal
  table.cell(
    rowspan: 2,
    align: center,
    fill: rgb("#2fcf40")
  )[
    #rotate(-90deg)[Ext crystal]
  ],
  [15], [XTAL1], [-], 
  [16], [XTAL2], [-],

  // USB-C data
  table.cell(
    rowspan: 2,
    align: center,
    fill: rgb("#ffc107")
  )[
    #rotate(-90deg)[USB-C data]
  ],
  [19], [D-], [-], 
  [20], [D+], [-],

  // IR receiver
  table.cell(
    rowspan: 1,
    align: center,
    fill: rgb("#ff0000")
  )[
    #rotate(-90deg)[IRM]
  ],
  [21], [IR receiver input], [-],

  // Shaft encoder
  table.cell(
    rowspan: 1,
    align: center,
    fill: rgb("#ff00bf")
  )[
    #rotate(-90deg)[AS \ 5600]
  ],
  [39], [Shaft encoder input], [-],

  // Debug LED
  table.cell(
    rowspan: 1,
    align: center,
    fill: rgb("#ff9900")
  )[
    #rotate(-90deg)[LED \ (red)]
  ],
  [42], [Debug LED output], [-],
)

#pagebreak()
== 4. Firmware Overview

The firmware is designed around a *main loop* that handles motor control,
sensor reading, and communication. Timing is not critical for typical NEMA17 motors,
as the ESP32-S2 can comfortably manage all tasks.
- *Motor Control*: Uses a stepper driver library for the TMC2209 or own control signals (#box[https://github.com/teemuatlut/TMCStepper]). 
- *TMC2209 UART Interface*: The MCU communicates with the driver over UART mainly to read stallGuard for sensorless homing, or to detect an obstacle.  
- *Shaft Encoder (AS5600)*: PWM output is sampled by the MCU to track shaft position.  
- *IR Receiver (IRM-H638T)*: Possible use for reasonably precise synchronization of multiple motors.
- *Communication*: The board can receive commands and send status updates over Wi-Fi.

== 5. Processing & Firmware Logic

All peripheral handling occurs in the *main loop*, with occasional use of interrupts for critical events such as stall flags from the TMC2209.  
The loop handles:  
- Stepper control updates  
- Encoder sampling  
- IR input reading  
- UART polling of the stepper driver (upon interrupt or otherwise)
- Wi-Fi

== 6. Communication Protocol

- The board listens for *high-level commands* (move steps, set speed, stop, etc.) via Wi-Fi.  
- *Status updates* (position, stall conditions, error flags) are sent back at a configurable rate.


#pagebreak()
== 7. Schematics

#pad(left: 1.5cm)[
  #rotate(-90deg, reflow: true)[
    #figure(
      image("schematics/v3.svg", width: 24cm),
      caption: [Complete Schematic]
    ) <complete_schematic>
  ]
]

#pagebreak()
=== KiCAD Hierarchical sheets \ (from complete schematic above)
#grid(
  columns: (1fr, 1fr),
  rows: (auto, 1fr),
  gutter: 80pt,


  grid.cell(
    colspan: 2,
    figure(
        image("schematics/v3-switching_regulator_TPS54331DR.svg"),
        caption: [switching_regulator_TPS54331DR]
    )
    ),

  grid.cell(
    colspan: 2,
    figure(
        image("schematics/v3-stepper_driver_tmc2209.svg"),
        caption: [stepper_driver_tmc2209]

    )
  ),

  grid.cell(
    colspan: 2,
    figure(
        image("schematics/v3-shaft_encoder_AS5600.svg"),
        caption: [shaft_encoder_AS5600]
    )
    
  ),
   grid.cell(
    figure(
        image("schematics/v3-irrec_IRM_H630T.svg"),
        caption: [irrec_IRM_H630T]
    )
    
  ),

  grid.cell(
    figure(
        image("schematics/v3-extern_crystal.svg"),
        caption: [External 32kHz crystal]
    )
    
  ),

)