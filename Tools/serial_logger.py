"""
@file serial_logger.py
@brief Automated Data Logger for Aether-Lock.
@details Reads telemetry from ESP32 via Serial and saves it to a CSV file
         in Hardware/Measurements/Data/Raw/ with a timestamp.
"""

import serial
import serial.tools.list_ports
import time
import os
import csv
from datetime import datetime

# --- CONFIGURATION ---
BAUD_RATE = 115200
# Relative path for data storage
OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "../Hardware/Measurements/Data/Raw")

def find_esp32_port():
    """Automatically detects the ESP32 COM port."""
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        # Search for common ESP32 USB-to-Serial drivers (CP210x or CH340)
        if "CP210" in p.description or "CH340" in p.description or "USB Serial" in p.description:
            return p.device
    return None

def main():
    # 1. Directory Setup
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    # 2. Port detection
    port = find_esp32_port()
    if not port:
        print("❌ ESP32 not found! Please connect the USB cable.")
        port = input("Enter port manually (e.g., COM3): ") 
    
    print(f"🔌 Connecting to {port} @ {BAUD_RATE} baud...")

    # 3. File Initialization
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"measure_{timestamp}.csv"
    filepath = os.path.join(OUTPUT_DIR, filename)

    try:
        with serial.Serial(port, BAUD_RATE, timeout=1) as ser, \
             open(filepath, mode='w', newline='') as csv_file:
            
            writer = csv.writer(csv_file)
            print(f"✅ Logging started! Saving to: {filename}")
            print("Press CTRL+C to stop.")
            print("-" * 40)

            while True:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                
                if line:
                    print(f"DATA: {line}")
                    
                    # If the line contains commas, treat it as CSV data
                    if "," in line:
                        # Strip labels or prefixes (e.g., ">Raw:100" becomes "100")
                        clean_parts = []
                        parts = line.split(',')
                        for p in parts:
                            if ':' in p: 
                                clean_parts.append(p.split(':')[1])
                            else:
                                clean_parts.append(p)
                        
                        writer.writerow(clean_parts)
                        csv_file.flush() # Ensure data is written to disk immediately

    except KeyboardInterrupt:
        print("\n🛑 Logging stopped by user.")
        print(f"File saved: {filepath}")

    except Exception as e:
        print(f"\n❌ Error: {e}")

if __name__ == "__main__":
    main()