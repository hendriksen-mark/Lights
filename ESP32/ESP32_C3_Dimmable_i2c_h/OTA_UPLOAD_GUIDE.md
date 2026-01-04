# OTA Firmware Upload via PlatformIO

Both `mesh_rotary` and `beetle_curtain` use PlatformIO's custom upload command to send firmware to the master device.

## Quick Start

```bash
# Build only (creates firmware file locally)
platformio run

# Build AND upload to master device  
platformio run --target upload
```

## Configuration

Edit `platformio.ini` in the `[common]` section:

```ini
[common]
master_device = ESP32_Light.local
; Alternative: use IP address like 192.168.1.100
```

**Master Device Options**:
- mDNS hostname: `ESP32_Light.local` (default, recommended)
- IP address: `192.168.1.100`

## Workflow

### Build Only
```bash
cd beetle_curtain  # or mesh_rotary
platformio run
```

**Output:**
```
✓ Created OTA firmware file:
  Name: firmware_ESP32_curtain.bin
  Size: 878,896 bytes
  Path: /path/to/firmware_ESP32_curtain.bin

To upload to master device:
  platformio run --target upload
```

Firmware file created locally - ready for manual upload if needed.

### Build + Upload to Master
```bash
cd beetle_curtain  # or mesh_rotary
platformio run --target upload
```

**Output (Success):**
```
✓ Created OTA firmware file
...
Uploading firmware_ESP32_curtain.bin to ESP32_Light.local...
Firmware size: 878,896 bytes
✓ Upload successful!
  Status: 302
```

**Output (Master Offline):**
```
✓ Created OTA firmware file
...
Uploading firmware_ESP32_curtain.bin to ESP32_Light.local...
✗ Upload failed: [Errno 8] nodename nor servname provided
  Make sure master device is accessible at ESP32_Light.local
*** [upload] Error 1
```

Note: Build succeeds even if upload fails - firmware file is still created locally.

## Complete Development Workflow

### Option 1: Direct USB Flash (Development)
```bash
# Edit code locally
cd beetle_curtain

# Flash directly to connected device via USB
platformio run --target upload

# Note: This uses USB upload, not HTTP to master
# Change upload_protocol in platformio.ini to use serial/espota
```

### Option 2: OTA via Master (Deployment)
```bash
# Edit code
cd mesh_rotary

# Build and upload to master in one command
platformio run --target upload

# Master now has firmware ready
# Navigate to http://<master-ip>/ota
# Start OTA broadcast
```

### Option 3: Build Locally, Upload Later
```bash
# Build when offline
platformio run

# Later, when master is available
platformio run --target upload
```

## Per-Project Configuration

### beetle_curtain
```ini
[common]
master_device = ESP32_Light.local

[env:dfrobot_beetle_esp32c3]
upload_protocol = custom
upload_command = python3 upload_to_master.py $PROJECT_DIR/firmware_ESP32_curtain.bin ${common.master_device} firmware_ESP32_curtain.bin
```

Always creates: `firmware_ESP32_curtain.bin`

### mesh_rotary
```ini
[common]
master_device = ESP32_Light.local

[env:esp01_1m]
upload_protocol = custom  
upload_command = python3 upload_to_master.py $PROJECT_DIR/firmware_ESP8266_*.bin ${common.master_device} firmware_ESP8266_*.bin
```

Creates firmware based on room in `include/config.h`:
- `firmware_ESP8266_keuken.bin`
- `firmware_ESP8266_slaapkamer.bin`
- etc.

## Changing Master Device

### Use Different Master for Each Environment
```bash
# Development master
cd mesh_rotary
# Edit platformio.ini: master_device = ESP32_Dev.local
platformio run --target upload

# Production master
# Edit platformio.ini: master_device = ESP32_Prod.local  
platformio run --target upload
```

### Use IP Address Instead of Hostname
```ini
[common]
master_device = 192.168.1.50
```

Useful when:
- mDNS not available
- Multiple devices with same hostname
- Network doesn't support .local resolution

## Troubleshooting

### "Make sure master device is accessible"

**Problem:** Cannot connect to master device

**Solutions:**
1. Check master is powered on and connected
   ```bash
   ping ESP32_Light.local
   ```

2. Try IP address instead of hostname
   ```ini
   master_device = 192.168.1.100
   ```

3. Verify same network
   ```bash
   arp -a | grep -i esp32
   ```

4. Check firewall/network settings

5. Build without upload:
   ```bash
   platformio run  # Just build
   # Upload manually via web interface
   ```

### "Upload failed: HTTP Error 404"

**Problem:** Master reachable but no `/ota/upload` endpoint

**Solutions:**
- Ensure master is running latest firmware with OTA support
- Check web server is running on master
- Verify endpoint exists: `http://ESP32_Light.local/ota`

### "Upload failed: HTTP Error 500"

**Problem:** Master received file but encountered error

**Solutions:**
- Check SD card mounted on master
- Verify `/firmware` directory exists and is writable
- Check master device logs via serial or log server

### Wrong Firmware File Uploaded

**Problem:** Wildcards in `upload_command` match wrong file

For `mesh_rotary`, ensure only one `firmware_ESP8266_*.bin` file exists:
```bash
cd mesh_rotary
rm firmware_ESP8266_*.bin  # Clean old builds
platformio run --target upload
```

## Advanced Configuration

### Custom Upload Script

The `upload_to_master.py` script can be customized:

```python
# upload_to_master.py
def upload_firmware(firmware_path, master_address, firmware_name):
    url = f"http://{master_address}/ota/upload"
    
    # Add custom headers
    req.add_header('X-Device-Type', 'curtain')
    
    # Add authentication if needed
    # req.add_header('Authorization', 'Bearer <token>')
```

### Environment-Specific Masters

```ini
[env:dev]
master_device = ESP32_Dev.local
...

[env:prod]
master_device = ESP32_Prod.local
...
```

Build for specific environment:
```bash
platformio run -e dev --target upload
platformio run -e prod --target upload
```

### CI/CD Integration

```bash
# In CI/CD pipeline
export MASTER_IP=192.168.1.100

# Modify platformio.ini via script
sed -i "s/master_device = .*/master_device = $MASTER_IP/" platformio.ini

# Build and upload
platformio run --target upload
```

## Files

- `platformio.ini` - Upload configuration (`master_device`, `upload_command`)
- `upload_to_master.py` - Upload script (HTTP POST multipart)
- `post_build.py` - Creates properly named firmware file after build
- `firmware_ESP32_curtain.bin` or `firmware_ESP8266_<room>.bin` - Generated firmware

## Benefits Over Manual Upload

✅ **Single Command** - `platformio run --target upload` does everything  
✅ **Version Control** - Master device setting in git-tracked `platformio.ini`  
✅ **Consistent** - Same naming convention every build  
✅ **Automated** - Perfect for CI/CD pipelines  
✅ **Standard** - Uses PlatformIO's native upload system  
✅ **Flexible** - Easy to switch between USB and HTTP upload
