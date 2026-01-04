# Automated OTA Firmware Generation

This project is configured to automatically generate properly named OTA firmware files after each build.

## How It Works

When you run `platformio run`, a post-build script (`post_build.py`) automatically:

1. **Detects Hardware**: ESP32 (beetle_curtain)
2. **Uses Device Name**: `curtain` (from DEVICE_NAME in config.h)
3. **Creates Firmware File**:
   - Copies the compiled firmware from `.pio/build/dfrobot_beetle_esp32c3/firmware.bin`
   - Renames it to: `firmware_ESP32_curtain.bin`
   - Places it in the project root directory
4. **Displays Instructions**: Shows OTA upload instructions

## Building Firmware

```bash
platformio run
```

This will compile the firmware and automatically create:
- `firmware_ESP32_curtain.bin`

## Build Output Example

```
============================================================
Creating OTA firmware file...
============================================================
✓ Created OTA firmware file:
  Name: firmware_ESP32_curtain.bin
  Device: curtain
  Hardware: ESP32
  Size: 878,896 bytes
  Path: /path/to/project/firmware_ESP32_curtain.bin

To use with OTA:
  1. Upload firmware_ESP32_curtain.bin to master device via web interface
  2. Start OTA broadcast for 'curtain' devices
============================================================
```

## Using with OTA

After building:

1. **Upload to Master Device**:
   - Navigate to `http://<master-ip>/ota`
   - Click "Upload New Firmware"
   - Select the generated `firmware_ESP32_curtain.bin` file

2. **Start OTA Broadcast**:
   - Enter `firmware_ESP32_curtain.bin` in the OTA page
   - Click "Start OTA Broadcast"
   - All beetle_curtain devices will receive the update

## Workflow

### Standard Development
```bash
# Make changes to code
# Build and flash directly to device via USB
platformio run --target upload

# When ready to deploy via OTA:
platformio run
# → firmware_ESP32_curtain.bin is created
# Upload via master device web interface
```

### Quick OTA Update to All Curtains

```bash
# Build firmware
platformio run

# Upload to master (example using curl)
curl -F "firmware=@firmware_ESP32_curtain.bin" \
  http://<master-ip>/ota/upload

# Start OTA broadcast (example using curl)
curl "http://<master-ip>/ota/start?file=firmware_ESP32_curtain.bin"
```

## Technical Details

- **Script**: `post_build.py` (PlatformIO extra script)
- **Trigger**: Runs automatically after successful build
- **Hardware**: ESP32 (ESP32-C3)
- **Device Name**: `curtain` (hardcoded, matches DEVICE_NAME in config.h)
- **File Location**: Project root directory
- **Automatic**: No manual configuration needed

## Notes

- The device name is always `curtain` for this project
- This matches the `DEVICE_NAME` defined in `include/config.h`
- All beetle_curtain devices use the same firmware file
- Each device will only accept updates if they match the device name
