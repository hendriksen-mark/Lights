#!/usr/bin/env python3
"""
Upload script for PlatformIO to send firmware to master device via HTTP
Usage: python upload_to_master.py <firmware_path> <master_address> <firmware_name>
"""
import sys
import urllib.request
import urllib.error
import urllib.parse
from io import BytesIO
import os

def upload_firmware(firmware_path, master_address, firmware_name):
    """Upload firmware file to master device via HTTP POST"""
    
    url = f"http://{master_address}/ota/upload"
    
    try:
        print(f"Uploading {firmware_name} to {master_address}...")
        print(f"URL: {url}")
        
        # Read the firmware file
        with open(firmware_path, 'rb') as f:
            firmware_data = f.read()
        
        file_size = len(firmware_data)
        print(f"Firmware size: {file_size:,} bytes")
        
        # Create multipart form data
        boundary = '----WebKitFormBoundary' + os.urandom(16).hex()
        body = BytesIO()
        
        # Add firmware file field
        body.write(f'--{boundary}\r\n'.encode())
        body.write(f'Content-Disposition: form-data; name="firmware"; filename="{firmware_name}"\r\n'.encode())
        body.write(b'Content-Type: application/octet-stream\r\n\r\n')
        body.write(firmware_data)
        body.write(b'\r\n')
        body.write(f'--{boundary}--\r\n'.encode())
        
        # Create request
        req = urllib.request.Request(url, data=body.getvalue())
        req.add_header('Content-Type', f'multipart/form-data; boundary={boundary}')
        
        # Send request
        with urllib.request.urlopen(req, timeout=30) as response:
            if response.status in [200, 302]:
                print(f"✓ Upload successful!")
                print(f"  Status: {response.status}")
                # After successful upload, instruct master to push firmware to slave
                try:
                    start_url = f"http://{master_address}/ota/start?file={urllib.parse.quote(firmware_name)}"
                    print(f"Triggering master to push firmware to slave: {start_url}")
                    with urllib.request.urlopen(start_url, timeout=15) as start_resp:
                        if start_resp.status == 200:
                            print("✓ Master triggered OTA to slave successfully.")
                            return 0
                        else:
                            print(f"✗ Master OTA trigger failed with status: {start_resp.status}")
                            return 1
                except urllib.error.URLError as e:
                    print(f"✗ Failed to trigger master OTA: {e}")
                    return 1
            else:
                print(f"✗ Upload failed with status: {response.status}")
                return 1
                
    except urllib.error.URLError as e:
        print(f"✗ Upload failed: {e}")
        print(f"  Make sure master device is accessible at {master_address}")
        return 1
    except FileNotFoundError:
        print(f"✗ Firmware file not found: {firmware_path}")
        return 1
    except Exception as e:
        print(f"✗ Upload error: {e}")
        return 1

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: upload_to_master.py <firmware_path> <master_address> <firmware_name>")
        sys.exit(1)
    
    firmware_path = sys.argv[1]
    master_address = sys.argv[2]
    firmware_name = sys.argv[3]
    
    sys.exit(upload_firmware(firmware_path, master_address, firmware_name))
