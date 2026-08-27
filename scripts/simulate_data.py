#!/usr/bin/env python3
"""
simulate_data.py

Generates a synthetic sensor log in the exact CSV format the firmware
prints over UART (millis,temp_c,pressure_hpa,humidity_pct). This lets you
develop and demo the plotting pipeline before your PCB exists, and gives
you a "known good" dataset to sanity-check the real hardware against once
it's built.

Usage:
    python3 simulate_data.py --minutes 60 --out ../data/sample_log.csv
"""
import argparse
import csv
import math
import random


def generate(minutes: int, interval_s: int, seed: int):
    random.seed(seed)
    rows = []
    base_temp = 22.0
    base_pressure = 1013.0
    base_humidity = 45.0

    n_samples = (minutes * 60) // interval_s
    for i in range(n_samples):
        t = i * interval_s
        # slow diurnal-ish drift + sensor noise, so plots look like real data
        drift = math.sin(2 * math.pi * i / max(n_samples, 1))
        temp = base_temp + 2.5 * drift + random.gauss(0, 0.08)
        pressure = base_pressure - 1.5 * drift + random.gauss(0, 0.05)
        humidity = base_humidity - 6.0 * drift + random.gauss(0, 0.3)
        humidity = max(0.0, min(100.0, humidity))

        rows.append([t * 1000, round(temp, 2), round(pressure, 2), round(humidity, 2)])
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--minutes", type=int, default=60, help="how many minutes of data to simulate")
    ap.add_argument("--interval", type=int, default=5, help="seconds between samples")
    ap.add_argument("--seed", type=int, default=42)
    ap.add_argument("--out", type=str, default="data/sample_log.csv")
    args = ap.parse_args()

    rows = generate(args.minutes, args.interval, args.seed)

    with open(args.out, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["millis", "temp_c", "pressure_hpa", "humidity_pct"])
        writer.writerows(rows)

    print(f"Wrote {len(rows)} simulated samples to {args.out}")


if __name__ == "__main__":
    main()
