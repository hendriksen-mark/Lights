Import("env")
import os
import shutil
import re

def post_build_action(source, target, env):
    """
    Post-build script to create properly named OTA firmware file.
    Reads the room define from include/config.h and creates:
    firmware_ESP8266_<room>.bin
    """
    
    print("=" * 60)
    print("Creating OTA firmware file...")
    print("=" * 60)
    
    # Read room name from config.h
    config_path = os.path.join(env.get("PROJECT_DIR"), "include", "config.h")
    room_name = None
    
    try:
        with open(config_path, 'r') as f:
            content = f.read()
            # Look for #define room <name> (not commented out)
            match = re.search(r'^\s*#define\s+room\s+(\w+)', content, re.MULTILINE)
            if match:
                room_name = match.group(1)
    except Exception as e:
        print(f"Error reading config.h: {e}")
        return
    
    if not room_name:
        print("WARNING: Could not find 'room' define in include/config.h")
        print("Skipping OTA firmware file creation")
        return
    
    # Get paths
    firmware_source = str(target[0])  # .pio/build/<env>/firmware.bin
    project_dir = env.get("PROJECT_DIR")
    
    # Create firmware filename: firmware_ESP8266_<room>.bin
    firmware_name = f"firmware_ESP8266_{room_name}.bin"
    firmware_dest = os.path.join(project_dir, firmware_name)
    
    # Copy firmware file
    try:
        shutil.copy2(firmware_source, firmware_dest)
        file_size = os.path.getsize(firmware_dest)
        print(f"✓ Created OTA firmware file:")
        print(f"  Name: {firmware_name}")
        print(f"  Room: {room_name}")
        print(f"  Size: {file_size:,} bytes")
        print(f"  Path: {firmware_dest}")
        print()
        print("To upload to master device:")
        print(f"  platformio run --target upload")
        print("Or manually via web interface at http://<master-ip>/ota")
        print("=" * 60)
    except Exception as e:
        print(f"ERROR: Failed to copy firmware: {e}")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", post_build_action)
