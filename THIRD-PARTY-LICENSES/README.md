# Third-Party Licenses

This directory contains licenses for third-party components used in the pico-altimeter project.

The main project code is licensed under the MIT License (see [LICENSE](../LICENSE) in the project root).

## Third-Party Components

| Component | License | License File |
|-----------|---------|--------------|
| BMP3 Sensor API (Bosch Sensortec) | BSD-3-Clause | [bosch-bmp3-sensor-api.txt](bosch-bmp3-sensor-api.txt) |

## Adding New Components

When adding a new third-party component:

1. Create a new text file in this directory named `<component-name>.txt`
2. Include the full license text from the component
3. Update the table above with the component name, license type, and filename
4. If the component requires attribution in binary distributions, add it to the NOTICES file

## License Compatibility

- **MIT License**: Compatible with BSD licenses
- **BSD 2-Clause**: Compatible with MIT
- **BSD 3-Clause**: Compatible with MIT

All BSD-licensed components are compatible with this project's MIT license.
