# TrailCurrent Shunt Gateway

Gateway module that reads battery monitoring data from a Victron BMV battery shunt via serial and transmits the readings over a CAN bus interface.

## Hardware Overview

- **Microcontroller:** ESP32
- **Function:** Serial-to-CAN bus bridge for Victron BMV battery shunt data
- **Key Features:**
  - Victron BMV VE.Direct serial protocol parsing
  - CAN bus output at 500 kbps
  - Battery voltage, current, SOC, and wattage reporting
  - Hierarchical PCB schematic design with FreeCAD enclosure
  - 8 LED status indicators

## Hardware Requirements

### Components

- **Microcontroller:** ESP32 development board
- **CAN Transceiver:** Vehicle CAN bus interface (TX: GPIO 15, RX: GPIO 13)
- **Serial Input:** Victron BMV VE.Direct connection (Serial2: TX GPIO 17, RX GPIO 16 at 19200 baud)
- **LED Indicators:** 8 status LEDs (GPIOs 32, 33, 26, 14, 23, 19, 17, 4)

### KiCAD Library Dependencies

This project uses the consolidated [TrailCurrentKiCADLibraries](https://codeberg.org/trailcurrentopensource/TrailCurrentKiCADLibraries).

**Setup:**

```bash
# Clone the library
git clone https://codeberg.org/trailcurrentopensource/TrailCurrentKiCADLibraries.git

# Set environment variables (add to ~/.bashrc or ~/.zshrc)
export TRAILCURRENT_SYMBOL_DIR="/path/to/TrailCurrentKiCADLibraries/symbols"
export TRAILCURRENT_FOOTPRINT_DIR="/path/to/TrailCurrentKiCADLibraries/footprints"
export TRAILCURRENT_3DMODEL_DIR="/path/to/TrailCurrentKiCADLibraries/3d_models"
```

See [KICAD_ENVIRONMENT_SETUP.md](https://codeberg.org/trailcurrentopensource/TrailCurrentKiCADLibraries/blob/main/KICAD_ENVIRONMENT_SETUP.md) in the library repository for detailed setup instructions.

## Opening the Project

1. **Set up environment variables** (see Library Dependencies above)
2. **Open KiCAD:**
   ```bash
   kicad EDA/trailer-shunt-can-bus.kicad_pro
   ```
3. **Verify libraries load** - All symbol and footprint libraries should resolve without errors
4. **View 3D models** - Open PCB and press `Alt+3` to view the 3D visualization

### Schematic Sheets

The design uses a hierarchical schematic with dedicated sheets:
- **Root** - Top-level connections
- **Power** - Power distribution and regulation
- **CAN** - CAN bus transceiver interface
- **MCU** - ESP32 microcontroller and support circuits
- **Connectivity** - Serial interface to Victron BMV

## Firmware

See `src/` directory for PlatformIO-based firmware.

**Setup:**
```bash
# Install PlatformIO (if not already installed)
pip install platformio

# Build firmware
pio run

# Upload to board
pio run -t upload
```

### Victron BMV Parameters

The firmware parses the following VE.Direct protocol fields:

| Parameter | Description |
|-----------|-------------|
| V | Battery voltage |
| I | Battery current (with sign) |
| P | Power/wattage (with sign) |
| SOC | State of charge |
| TTG | Time to go |
| CE | Consumed energy |
| H1-H12, H17-H18 | Historical data |

### CAN Bus Protocol

The gateway transmits two messages at 500 kbps with a 33ms update cycle:

**Message 0x23** (7 bytes) - Battery basics:

| Byte | Description |
|------|-------------|
| 0 | Battery voltage (whole) |
| 1 | Battery voltage (decimal) |
| 2 | Current sign |
| 3 | Current (whole) |
| 4 | Current (decimal) |
| 5 | SOC (whole) |
| 6 | SOC (decimal) |

**Message 0x24** (5 bytes) - Power metrics:

| Byte | Description |
|------|-------------|
| 0 | Wattage sign |
| 1 | Wattage MSB |
| 2 | Wattage LSB |
| 3 | TTG MSB |
| 4 | TTG LSB |

## Manufacturing

- **PCB Files:** Ready for fabrication via standard PCB services (JLCPCB, OSH Park, etc.)
- **BOM Generation:** Export BOM from KiCAD schematic (Tools > Generate BOM)
- **Enclosure:** FreeCAD design included in `CAD/` directory
- **JLCPCB Assembly:** See [BOM_ASSEMBLY_WORKFLOW.md](https://codeberg.org/trailcurrentopensource/TrailCurrentKiCADLibraries/blob/main/BOM_ASSEMBLY_WORKFLOW.md) for detailed assembly workflow

## Project Structure

```
├── CAD/                          # FreeCAD enclosure design
├── EDA/                          # KiCAD hardware design files
│   ├── trailer-shunt-can-bus.kicad_pro
│   ├── trailer-shunt-can-bus.kicad_sch  # Root schematic
│   ├── can.kicad_sch             # CAN subsystem
│   ├── connectivity.kicad_sch    # Serial interface
│   ├── mcu.kicad_sch             # MCU subsystem
│   ├── power.kicad_sch           # Power subsystem
│   └── trailer-shunt-can-bus.kicad_pcb  # PCB layout
├── src/                          # Firmware source
│   ├── main.cpp                  # Victron BMV parser and CAN transmitter
│   ├── globals.h                 # Pin definitions and debug macros
│   └── canHelper.h               # CAN bus configuration
└── platformio.ini                # Build configuration
```

## License

MIT License - See LICENSE file for details.

This is open source hardware. You are free to use, modify, and distribute these designs under the terms of the MIT license.

## Contributing

Improvements and contributions are welcome! Please submit issues or pull requests.

## Support

For questions about:
- **KiCAD setup:** See [KICAD_ENVIRONMENT_SETUP.md](https://codeberg.org/trailcurrentopensource/TrailCurrentKiCADLibraries/blob/main/KICAD_ENVIRONMENT_SETUP.md)
- **Assembly workflow:** See [BOM_ASSEMBLY_WORKFLOW.md](https://codeberg.org/trailcurrentopensource/TrailCurrentKiCADLibraries/blob/main/BOM_ASSEMBLY_WORKFLOW.md)
