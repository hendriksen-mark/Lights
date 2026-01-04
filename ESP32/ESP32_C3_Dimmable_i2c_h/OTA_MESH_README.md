# Mesh OTA Update System

## Overview

This project now supports Over-The-Air (OTA) firmware updates for mesh slave devices (beetle_curtain and mesh_rotary) via the painlessMesh network. The master device (running this firmware) can broadcast firmware updates to specific device types over the mesh network.

## Device Types

### Master Device
- **Device**: ESP32_C3_Dimmable_i2c_h (this project)
- **Role**: OTA Sender
- **Functions**: 
  - Stores firmware files on SD card
  - Broadcasts OTA updates via mesh
  - Web interface for OTA management

### Slave Devices

#### Beetle Curtain
- **Device Name**: `curtain`
- **OTA Receiver ID**: `DEVICE_NAME` = `"curtain"`
- **Firmware File**: `firmware_ESP32_curtain.bin`

#### Mesh Rotary
- **Device Name**: Room-specific (e.g., `keuken`, `slaapkamer`, `woonkamer`, `gang`, `badkamer`)
- **OTA Receiver ID**: `String(room)` where room is defined in config
- **Firmware Files**: 
  - `firmware_ESP32_keuken.bin` (kitchen)
  - `firmware_ESP32_slaapkamer.bin` (bedroom)
  - `firmware_ESP32_woonkamer.bin` (living room)
  - `firmware_ESP32_gang.bin` (hallway)
  - `firmware_ESP32_badkamer.bin` (bathroom)
  - `firmware_ESP32_gang_beweging.bin` (hallway motion)
  - `firmware_ESP32_badkamer_beweging.bin` (bathroom motion)

## Firmware File Naming Convention

**Format**: `firmware_<HARDWARE>_<DEVICE_NAME>.bin`

- **HARDWARE**: Must be `ESP32` or `ESP8266` (case-sensitive)
- **DEVICE_NAME**: The device identifier used in `mesh.initOTAReceive()`

### Examples
```
firmware_ESP32_curtain.bin          # For beetle_curtain devices
firmware_ESP32_keuken.bin           # For mesh_rotary in kitchen
firmware_ESP32_slaapkamer.bin       # For mesh_rotary in bedroom
firmware_ESP32_gang_beweging.bin    # For mesh_rotary motion detector in hallway
```

## Preparing Firmware Files

### 1. Build Firmware for Slave Devices

#### For beetle_curtain:
```bash
cd beetle_curtain
platformio run
# Output: .pio/build/<env>/firmware.bin
```

#### For mesh_rotary:
```bash
cd mesh_rotary
# Edit include/config.h to set the room
platformio run
# Output: .pio/build/<env>/firmware.bin
```

### 2. Rename and Copy to SD Card

Rename the compiled firmware following the naming convention and place in `/firmware` directory on the SD card:

```
SD Card Structure:
├── firmware/
│   ├── firmware_ESP32_curtain.bin
│   ├── firmware_ESP32_keuken.bin
│   ├── firmware_ESP32_slaapkamer.bin
│   └── ... (other room firmwares)
├── mesh_config.json
└── ... (other config files)
```

**Important**: The `/firmware` directory will be automatically created when uploading via web interface.

## Using the OTA System

### Web Interface Access

1. Navigate to the mesh master device IP in your browser
2. Click the **"OTA Update"** button on the main page
3. You'll see the OTA management page with:
   - Current OTA status
   - List of available firmware files
   - Upload new firmware form
   - Start OTA broadcast form

### Uploading Firmware Files

**Via Web Interface** (Recommended):
1. Go to the OTA page (`http://<device-ip>/ota`)
2. Scroll to "Upload New Firmware" section
3. Select your `.bin` file (must follow naming convention)
4. Click "Upload"
5. File will be automatically placed in `/firmware` directory

**Via SD Card Direct Copy**:
1. Remove SD card from master device
2. Create `/firmware` directory if it doesn't exist
3. Copy renamed firmware files to `/firmware` directory
4. Reinsert SD card

### Starting an OTA Update

1. Open the OTA page (`http://<device-ip>/ota`)
2. Under "Start OTA Update" section:
   - Enter the filename (e.g., `firmware_ESP32_curtain.bin`)
   - Click "Start OTA Broadcast"
3. The master will:
   - Calculate MD5 hash of the firmware
   - Start broadcasting OTA availability to mesh
   - Continue broadcasting for 60 minutes

### Monitoring OTA Progress

The OTA status section shows:
- **Active**: Whether an OTA broadcast is currently running
- **Firmware**: The firmware file being broadcast
- **Device**: The target device name
- **Hardware**: The target hardware type

### Stopping an OTA Broadcast

If you need to stop an OTA broadcast before the 60-minute timeout:
1. Click the **"Stop OTA"** button on the OTA page
2. The broadcast will immediately stop

## API Endpoints

### GET /ota
Main OTA management page (web interface)

### GET /ota/list
Returns plain text list of available firmware files

### GET /ota/start?file=<filename>
Start OTA broadcast for specified firmware
- **Parameter**: `file` - filename in `/firmware` directory
- **Example**: `/ota/start?file=firmware_ESP32_curtain.bin`

### GET /ota/stop
Stop current OTA broadcast

### GET /ota/status
Returns JSON with current OTA status:
```json
{
  "active": true/false,
  "firmware": "firmware_ESP32_curtain.bin",
  "device": "curtain",
  "hardware": "ESP32"
}
```

### POST /ota/upload
Upload new firmware file
- **Content-Type**: `multipart/form-data`
- **Field**: `firmware` (file input)
- **Accepts**: `.bin` files only

## How It Works

### OTA Broadcast Process

1. **Initialization**: 
   - Master reads firmware file from SD card
   - Calculates MD5 hash for integrity verification
   - Initializes data packet handler

2. **Broadcasting**:
   - Sends OTA announcement every 60 seconds for 60 minutes
   - Announcement contains: device name, hardware type, MD5 hash, number of packets

3. **Slave Response**:
   - Slaves with matching device name receive announcement
   - Slave compares MD5 with current firmware
   - If different, slave requests firmware packets

4. **Data Transfer**:
   - Master sends 1KB packets on demand
   - Slave assembles packets and verifies MD5
   - Slave flashes new firmware and reboots

### Technical Details

- **Packet Size**: 1024 bytes (configurable via `OTA_PART_SIZE`)
- **Broadcast Duration**: 60 minutes (60 iterations, 1 per minute)
- **Data Transfer**: On-demand via mesh network
- **Integrity Check**: MD5 hash verification
- **File Storage**: SD card (`/firmware` directory)

## Slave Device Configuration

### For Existing Slaves
Ensure your slave devices have OTA receive capability initialized:

**beetle_curtain** (already configured):
```cpp
mesh.initOTAReceive(DEVICE_NAME);  // DEVICE_NAME = "curtain"
```

**mesh_rotary** (already configured):
```cpp
mesh.initOTAReceive(String(room));  // room is defined in config.h
```

### Important Notes

⚠️ **CRITICAL**: Make sure any firmware you upload via OTA includes OTA support! If you flash firmware without `mesh.initOTAReceive()`, you will lose the ability to update that device over the air and will need to physically reflash it.

✅ The current beetle_curtain and mesh_rotary firmwares already include OTA support.

## Troubleshooting

### Firmware File Not Found
- Check filename follows exact format: `firmware_ESP32_<device>.bin`
- Verify file exists in `/firmware` directory on SD card
- Check SD card is properly mounted

### OTA Not Working
- Ensure target devices are connected to mesh (check mesh node list)
- Verify device name matches exactly (case-sensitive)
- Check target device has OTA receive initialized
- Ensure mesh network is stable

### Upload Fails
- Maximum upload size is 300KB (configurable via `MAX_UPLOAD_SIZE`)
- Ensure filename follows naming convention
- Check SD card has sufficient space

### Device Name Mismatch
- Device names are case-sensitive
- Check slave device configuration for exact device name
- For mesh_rotary, check the `room` define in `include/config.h`

## Example Workflow

### Updating All Kitchen Rotary Switches

1. **Build firmware**:
   ```bash
   cd mesh_rotary
   # Edit include/config.h: #define room keuken
   platformio run
   ```

2. **Prepare firmware**:
   ```bash
   cp .pio/build/<env>/firmware.bin firmware_ESP32_keuken.bin
   ```

3. **Upload to master**:
   - Open web interface: `http://<master-ip>/ota`
   - Upload `firmware_ESP32_keuken.bin`

4. **Start OTA**:
   - Enter filename: `firmware_ESP32_keuken.bin`
   - Click "Start OTA Broadcast"

5. **Wait for updates**:
   - All mesh_rotary devices with `room = keuken` will receive update
   - Devices will reboot with new firmware automatically

### Updating Curtain Controllers

1. Build and rename: `firmware_ESP32_curtain.bin`
2. Upload via web interface
3. Start OTA broadcast
4. All beetle_curtain devices will update

## Remote Logging

OTA operations are logged via the remote logging system. Check the log server (port 2001) for detailed OTA activity:

```
INFO: Starting OTA for device: curtain hardware: ESP32 size: 234567
INFO: Firmware MD5: a1b2c3d4e5f6...
INFO: OTA broadcast started. Will send for 60 minutes.
```

## Security Considerations

- MD5 hash verification ensures firmware integrity
- Only devices with matching device names receive updates
- Network is encrypted with mesh password (`MESH_PASSWORD`)
- Web interface accessible to anyone on network (consider network security)

## Future Enhancements

Possible improvements:
- [ ] Firmware version tracking
- [ ] Scheduled OTA updates
- [ ] Progress tracking per device
- [ ] Rollback capability
- [ ] OTA history log
- [ ] Authentication for OTA endpoints
