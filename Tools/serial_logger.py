"""
@file serial_logger.py
@brief Professional Telemetry Logger for Aether-Lock.
@details Handles serial communication with ESP32 to log distance calibration 
         and coil linearity tests into CSV format.
"""

import serial
import serial.tools.list_ports
import time
import os
import csv
from datetime import datetime

# --- CONFIGURATION ---
BAUD_RATE = 115200
DEFAULT_OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "../Hardware/Measurements/Data/Raw")

def find_esp32_port():
    """Detects the first available ESP32 based on common USB-to-Serial descriptors."""
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        if any(driver in p.description for driver in ["CP210", "CH340", "USB Serial"]):
            return p.device
    return None

def get_logging_setup():
    """Handles terminal UI for mode selection and file naming."""
    print("\n" + "="*30)
    print("   AETHER-LOCK DATA LOGGER")
    print("="*30)
    print("1. Distance Measurement (Static - 50Hz)")
    print("2. Coil Linearity Test (PWM Ramp 0->100%)")
    
    choice = input("\nSelect operating mode [1/2]: ").strip()

    if choice == '1':
        dist = input("Enter distance label (e.g., '1.5cm', 'inf'): ").strip()
        return f"dist_{dist}.csv", b'1', f"DISTANCE MODE: Position magnet at {dist}."
    elif choice == '2':
        return "coil_linearity_test.csv", b'2', "RAMP MODE: Ensure magnet is removed."
    else:
        return None, None, None

def main():
    os.makedirs(DEFAULT_OUTPUT_DIR, exist_ok=True)
    
    port = find_esp32_port()
    if not port:
        print("❌ Error: ESP32 not found. Check physical connection.")
        return

    filename, cmd, desc = get_logging_setup()
    if not filename:
        print("❌ Invalid selection. Exiting.")
        return

    filepath = os.path.join(DEFAULT_OUTPUT_DIR, filename)
    print(f"\n{desc}")
    print(f"🔌 Initializing connection on {port}...")

    try:
        with serial.Serial(port, BAUD_RATE, timeout=1) as ser, \
             open(filepath, mode='w', newline='') as csv_file:
            
            writer = csv.writer(csv_file)
            
            # Allow time for ESP32 serial buffer to stabilize after connection
            time.sleep(2) 
            
            print(f"🚀 Sending command '{cmd.decode()}'...")
            ser.write(cmd)
            
            print(f"✅ Logging active: {filename}")
            print("Press CTRL+C to terminate session.")
            
            # Write standardized CSV Header
            writer.writerow(["Time_ms", "PWM_Percent", "Raw_ADC"])

            while True:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                
                if line:
                    # Filter for data lines (containing commas) and ignore header echoes
                    if "," in line and "Time" not in line: 
                        print(f" [REC] {line}")
                        writer.writerow(line.split(','))
                        csv_file.flush()
                    
                    # Check for completion flags from firmware
                    elif any(stop_msg in line for stop_msg in ["STOPPED", "FINISHED"]):
                        print("\n⏹️  Firmware signaled test completion.")
                        break
                    
                    # Display debug or informational messages from ESP32
                    else:
                        print(f" [ESP] {line}")

    except KeyboardInterrupt:
        print("\n🛑 Termination requested by user.")
        # Attempt to send stop command to ESP32 for hardware safety
        try:
            with serial.Serial(port, BAUD_RATE, timeout=0.5) as ser:
                ser.write(b'0')
        except:
            pass
        
    except Exception as e:
        print(f"\n❌ Critical Error: {e}")

if __name__ == "__main__":
    main()