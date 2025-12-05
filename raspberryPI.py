import serial
from datetime import datetime
import os
import glob

# Auto-detect the correct USB port
ports = glob.glob('/dev/ttyUSB*') + glob.glob('/dev/ttyACM*')
if len(ports) == 0:
    raise Exception("No serial device found!")
PORT = ports[0]

BAUD = 115200

# Create a unique text file for each session
timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
OUT_FILE = f"braille_output_{timestamp}.txt"

print(f"Listening on {PORT} @ {BAUD} ... (Stop: Ctrl+F2 in Thonny)")
print(f"Writing to: {OUT_FILE}")

# Open serial connection
ser = serial.Serial(PORT, BAUD, timeout=1)

# Open text file
with open(OUT_FILE, "a", encoding="utf-8", buffering=1) as f:
    # Session header
    f.write(f"\n--- New session @ {datetime.now().isoformat(timespec='seconds')} ---\n")
    f.flush()

    count = 0

    try:
        while True:
            b = ser.read(1)       # read single byte
            if not b:
                continue

            ch = b.decode(errors="ignore")  # decode safely

            # Print to screen
            print(ch, end="", flush=True)

            # Write to file
            f.write(ch)
            count += 1

            # Every 32 chars → force full save
            if count % 32 == 0:
                f.flush()
                os.fsync(f.fileno())

    except KeyboardInterrupt:
        print("\nStopped by user.")

    finally:
        try:
            f.flush()
            os.fsync(f.fileno())
        except:
            pass
        ser.close()
