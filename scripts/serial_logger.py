#!/usr/bin/env python3
"""
serial_logger.py

Reads live CSV lines from the board over USB-serial (via your USB-to-UART
adapter) and saves them to a file. Once you've built the hardware, this
replaces simulate_data.py as your real data source.

Usage:
    pip install pyserial
    python3 serial_logger.py --port /dev/ttyUSB0 --baud 115200 --out data/live_log.csv

On Windows, --port will look like COM5. On macOS, something like
/dev/tty.usbserial-XXXX.
"""
import argparse
import sys

try:
    import serial
except ImportError:
    print("This script needs pyserial: pip install pyserial")
    sys.exit(1)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", required=True, help="e.g. /dev/ttyUSB0 or COM5")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--out", default="data/live_log.csv")
    args = ap.parse_args()

    ser = serial.Serial(args.port, args.baud, timeout=2)
    print(f"Listening on {args.port} @ {args.baud} baud. Ctrl+C to stop.")

    with open(args.out, "w", buffering=1) as f:
        try:
            while True:
                line = ser.readline().decode("utf-8", errors="ignore").strip()
                if not line:
                    continue
                print(line)
                f.write(line + "\n")
        except KeyboardInterrupt:
            print(f"\nStopped. Saved log to {args.out}")


if __name__ == "__main__":
    main()
