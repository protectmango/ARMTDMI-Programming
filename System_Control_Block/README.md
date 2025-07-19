# System Control Block (LPC2129)

The System Control Block in the LPC2129 microcontroller is a collection of **system features** and **control registers** that manage various functions **not directly** tied to specific peripheral devices. 

These functions include:

- **Crystal Oscillator**  
 Manages the main clock source for the microcontroller.

- **External Interrupt Inputs**   
 Handles external interrupt requests.

- **Memory Mapping Control**   
Controls how different memory regions are mapped in the address space.

- **PLL (Phase Locked Loop)**  
Generates **higher frequency clocks** from the main oscillator for the **CPU and other components**.

- **Power Control**  
Manages the chip's power modes
    - including **Idle** and **Power Down** modes
    - allows **enabling or disabling** power to individual peripherals.

- **Reset**  
Handles various reset sources for the microcontroller.

- **VPB Divider**   
Controls the clock speed for the **VLSI Peripheral Bus (VPB)**.

- **Wakeup Timer**  
Used in **conjunction** with **reset** to ensure proper chip **startup**.   
Can also wake the device from **low-power** modes.


> Each of these functions has its own dedicated registers within the System Control Block for configuration and control.   

## Pin Description 

|Pin name| Pin direction |Pin Description|
|---|---|---|
|X1 |Input| **Crystal Oscillator Input**- Input to the oscillator and internal clock generator circuits.
|X2 |Output| **Crystal Oscillator Output**- Output from the oscillator amplifier.|
|EINT0 |Input|**External Interrupt Input 0** - An active low general purpose interrupt input. This pin may be used to wake up the processor from Idle or Power down modes. **Pins P0.1 and P0.16** can be selected to perform **EINT0** function. LOW level on this pin immediately after reset is considered as an external hardware request to start the ISP command handler. More details on ISP and Flash memory can be found in **"Flash Memory System and Programming"** chapter.|
|EINT1 |Input|**External Interrupt Input 1**- See the EINT0 description above.Pins P0.3 and P0.14 can be selected to perform EINT1 function.|
|EINT2 |Input|**External Interrupt Input 2**- See the EINT0 description above.Pins P0.7 and P0.15 can be selected to perform EINT2 function.|
|EINT3 |Input|**External Interrupt Input 3**- See the EINT0 description above.Pins P0.9, P0.20 and P0.30 can be selected to perform EINT3 function.|
|RESET |Input|External Reset input- A low on this pin resets the chip, causing I/O ports and peripherals to take on their default states, and the processor to begin execution at address 0.|


## Sytem Control Registers


| Name       | Description                                  | Access | Reset Value* | Address       |
|:-----------|:--------------------------------------------|:------:|:------------:|:-------------:|
| **External Interrupts**                                                                 |
| EXTINT     | External Interrupt Flag Register              | R/W    | 0            | 0xE01FC140    |
| EXTWAKE    | External Interrupt Wakeup Register            | R/W    | 0            | 0xE01FC144    |
| EXTMODE    | External Interrupt Mode Register              | R/W    | 0            | 0xE01FC148    |
| EXTPOLAR   | External Interrupt Polarity Register         | R/W    | 0            | 0xE01FC14C    |
| **Memory Mapping Control**                                                              |
| MEMMAP     | Memory Mapping Control                       | R/W    | 0            | 0xE01FC040    |
| **Phase Locked Loop**                                                                   |
| PLLCON     | PLL Control Register                         | R/W    | 0            | 0xE01FC080    |
| PLLCFG     | PLL Configuration Register                   | R/W    | 0            | 0xE01FC084    |
| PLLSTAT    | PLL Status Register                          | RO     | 0            | 0xE01FC088    |
| PLLFEED    | PLL Feed Register                            | WO     | NA           | 0xE01FC08C    |
| **Power Control**                                                                       |
| PCON       | Power Control Register                       | R/W    | 0            | 0xE01FC0C0    |
| PCONP      | Power Control for Peripherals                 | R/W    | 0x3BE        | 0xE01FC0C4    |
| **VPB Divider**                                                                         |
| VPBDIV     | VPB Divider Control                          | R/W    | 0            | 0xE01FC100    |

