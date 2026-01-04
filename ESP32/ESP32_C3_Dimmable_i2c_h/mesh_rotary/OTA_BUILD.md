# Automated OTA Firmware Generation

This project is configured to automatically generate properly named OTA firmware files after each build.

## How It Works

When you run `platformio run`, a post-build script (`post_build.py`) automatically:

1. **Reads Configuration**: 
   - Detects the room name from `include/config.h`
   - Identifies the hardware type (ESP8266)

2. **Creates Firmware File**:
   - Copies the compiled firmware from `.pio/build/esp01_1m/firmware.bin`
   - Renames it to: `firmware_ESP8266_<room>.bin`
   - Places it in the project root directory

3. **Displays Instructions**:
   - Shows the generated filename
   - Provides OTA upload instructions

## Building Firmware

### Standard Build
```bash
platformio run
```

This will compile the firmware and automatically create:
- `firmware_ESP8266_<room>.bin` (e.g., `firmware_ESP8266_keuken.bin`)

### Changing the Room

1. Edit `include/config.h`
2. Uncomment the desired room:
   ```cpp
   // #define room slaapkamer
   // #define room woonkamer
   #define room keuken        // ← Active room
   // #define room gang
   // #define room badkamer
   ```
3. Run `platformio run`
4. The firmware will be named according to the active room

## Available Room Names

| Room Define | Firmware Name | Description |
|-------------|---------------|-------------|
| `keuken` | `firmware_ESP8266_keuken.bin` | Kitchen |
| `slaapkamer` | `firmware_ESP8266_slaapkamer.bin` | Bedroom |
| `woonkamer` | `firmware_ESP8266_woonkamer.bin` | Living room |
| `gang` | `firmware_ESP8266_gang.bin` | Hallway |
| `badkamer` | `firmware_ESP8266_badkamer.bin` | Bathroom |
| `gang_beweging` | `firmware_ESP8266_gang_beweging.bin` | Hallway motion |
| `badkamer_beweging` | `firmware_ESP8266_badkamer_beweging.bin` | Bathroom motion |

## Using with OTA

After building:

1. **Upload to Master Device**:
   - Navigate to `http://<master-ip>/ota`
   - Click "Upload New Firmware"
   - Select the generated `firmware_ESP8266_<room>.bin` file

2. **Start OTA Broadcast**:
   - Enter the filename in the OTA page
   - Click "Start OTA Broadcast"
   - All devices with matching room name will receive the update

## Build Output Example

```
============================================================
Creating OTA firmware file...
============================================================
✓ Created OTA firmware file:
  Name: firmware_ESP8266_keuken.bin
  Room: keuken
  Size: 380,368 bytes
  Path: /path/to/project/firmware_ESP8266_keuken.bin

To use with OTA:
  1. Upload firmware_ESP8266_keuken.bin to master device via web interface
  2. Start OTA broadcast for 'keuken' devices
============================================================
```

## Multiple Room Firmwares

To build firmware for multiple rooms:

```bash
# Build for kitchen
sed -i '' 's/#define room.*/#define room keuken/' include/config.h
platformio run

# Build for bedroom  
sed -i '' 's/#define room.*/#define room slaapkamer/' include/config.h
platformio run

# Build for living room
sed -i '' 's/#define room.*/#define room woonkamer/' include/config.h
platformio run
```

This will create:
- `firmware_ESP8266_keuken.bin`
- `firmware_ESP8266_slaapkamer.bin`
- `firmware_ESP8266_woonkamer.bin`

## Troubleshooting

### "Could not find 'room' define"

Make sure `include/config.h` has an uncommented `#define room <name>` line:

```cpp
// BAD - all commented out
// #define room keuken
// #define room slaapkamer

// GOOD - one uncommented
#define room keuken
// #define room slaapkamer
```

### Wrong Room Name

Check that the room name in `include/config.h` matches exactly (case-sensitive):
- `keuken` ✓
- `Keuken` ✗
- `KEUKEN` ✗

## Technical Details

- **Script**: `post_build.py` (PlatformIO extra script)
- **Trigger**: Runs automatically after successful build
- **Hardware Detection**: Always `ESP8266` for this project
- **Room Detection**: Parses `#define room <name>` from `include/config.h`
- **File Location**: Project root directory
