## Characteristics

* Microcontroller Puya PY32F002A    
  * PY32F002Ax5(20KB Flash/3KB RAM) SPI, USART, I2C, ADC.

## License
<a>
  <img src="https://github.com/ElectronicCats/AjoloteBoard/raw/master/OpenSourceLicense.png" height="150" />
</a>

Designed by Electronic Cats for HackGDL

Electronic Cats invests time and resources in providing this open-source design. Please support Electronic Cats and open-source hardware by purchasing products from Electronic Cats!

Hardware released under ...

# PY32F0 SDK reference for Windows

[How to install the SDK for windows](https://github.com/ElectronicCats/puya-projects)

* Puya PY32F0 seriees template project for GNU Arm Embedded Toolchain
* Supported programmers: J-Link
* Supported IDE: VSCode

# File Structure

```
├── Build                       # Build results
├── Docs                        # Datesheets and User Manuals
├── Examples
│   ├── FreeRTOS                # FreeRTOS examples
│   ├── HAL                     # HAL library examples
│   └── LL                      # LL(Low Layer) library examples
├── Libraries
│   ├── BSP                     # SysTick delay and printf for debug
│   ├── BSP_LL                  # SysTick delay and printf for debug
│   ├── CMSIS
│   ├── FreeRTOS                # FreeRTOS library
│   ├── LDScripts               # LD files
│   ├── PY32F0xx_HAL_Driver     # MCU peripheral driver
│   └── PY32F0xx_LL_Driver      # MCU low layer peripheral driver
├── Makefile                    # Make config
├── Misc
│   ├── Flash
│   │   ├── JLinkDevices        # JLink flash loaders
│   │   └── Sources             # Flash algorithm source code
│   ├── Puya.PY32F0xx_DFP.x.pack # DFP pack file for PyOCD
│   └── SVD                     # SVD files
├── README.md
├── rules.mk                    # Pre-defined rules include in Makefile 
└── User                        # User application code
```

# Requirements

* Programmer
  * J-Link: J-Link OB programmer: https://linhkienthuduc.com/jlink-ob-072-mach-nap-cho-arm-cortex-m
* SEGGER J-Link Software and Documentation pack [https://www.segger.com/downloads/jlink/](https://www.segger.com/downloads/jlink/)
* GNU Arm Embedded Toolchain
* Make 

# Try Other Examples

[Forked Repository](https://github.com/IOsetting/py32f0-template)
More examples can be found in *Examples* folder of the forked repository, copy and replace the files under *User* folder of this repository to try different examples.
