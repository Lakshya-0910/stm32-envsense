#!/usr/bin/env python3
"""
plot_data.py

Reads a CSV log (from simulate_data.py or serial_logger.py) and produces
a 3-panel PNG chart: temperature, pressure, humidity over time. This is
what you'd embed in your README as proof the board works.

Usage:
    python3 plot_data.py --csv data/sample_log.csv --out data/plot.png
"""
import argparse

import matplotlib
matplotlib.use("Agg")  # headless-safe, works in CI
import matplotlib.pyplot as plt
import pandas as pd


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", default="data/sample_log.csv")
    ap.add_argument("--out", default="data/plot.png")
    args = ap.parse_args()

    df = pd.read_csv(args.csv)
    df["minutes"] = df["millis"] / 1000 / 60

    fig, axes = plt.subplots(3, 1, figsize=(9, 8), sharex=True)

    axes[0].plot(df["minutes"], df["temp_c"], color="#d1495b")
    axes[0].set_ylabel("Temp (\u00b0C)")
    axes[0].set_title("EnvSense Board Log")

    axes[1].plot(df["minutes"], df["pressure_hpa"], color="#3d5a80")
    axes[1].set_ylabel("Pressure (hPa)")

    axes[2].plot(df["minutes"], df["humidity_pct"], color="#2a9d8f")
    axes[2].set_ylabel("Humidity (%RH)")
    axes[2].set_xlabel("Time (minutes)")

    for ax in axes:
        ax.grid(alpha=0.3)

    fig.tight_layout()
    fig.savefig(args.out, dpi=150)
    print(f"Saved plot to {args.out}")

    print("\nSummary stats:")
    print(df[["temp_c", "pressure_hpa", "humidity_pct"]].describe().round(2))


if __name__ == "__main__":
    main()
