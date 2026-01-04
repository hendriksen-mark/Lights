Import("env")
import os
import shutil

def post_build_action(source, target, env):
    """
    Post-build script to create properly named OTA firmware file.
    Creates: firmware_ESP32_curtain.bin
    """
    
    print("=" * 60)
    print("Creating OTA firmware file...")
    print("=" * 60)
    
    # Device name is always "curtain" for beetle_curtain
    device_name = "curtain"
    hardware = "ESP32"
    
    # Get paths
    firmware_source = str(target[0])  # .pio/build/<env>/firmware.bin
    project_dir = env.get("PROJECT_DIR")
    
    # Create firmware filename: firmware_ESP32_curtain.bin
    firmware_name = f"firmware_{hardware}_{device_name}.bin"
    firmware_dest = os.path.join(project_dir, firmware_name)
    
    # Copy firmware file
    try:
        shutil.copy2(firmware_source, firmware_dest)
        file_size = os.path.getsize(firmware_dest)
        print(f"✓ Created OTA firmware file:")
        print(f"  Name: {firmware_name}")
        print(f"  Device: {device_name}")
        print(f"  Hardware: {hardware}")
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
