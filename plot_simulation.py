import matplotlib.pyplot as plt
import numpy as np
import os

# Settings
processor_counts = [2, 4, 8, 16, 32, 64]
num_modules = np.arange(1, 513)
distributions = ['uniform', 'normal']
vlines = [64, 128, 256]
saturation_threshold = 0.01
stability_window = 10

def load_data(dist, procs):
    filename = f"{dist}_{procs}.csv"
    if not os.path.exists(filename):
        print(f"Missing: {filename}")
        return None
    with open(filename) as f:
        values = [float(line.strip()) for line in f if line.strip()]
    return values if len(values) == 512 else None

def find_saturation_point(values):
    deltas = [abs(values[i+1] - values[i]) / values[i] for i in range(len(values) - 1)]
    for i in range(len(deltas) - stability_window):
        if all(d < saturation_threshold for d in deltas[i:i+stability_window]):
            return i + 1  # index to memory module
    return None

def plot_distribution(dist):
    plt.figure(figsize=(10, 6))
    legend_labels = []

    for p in processor_counts:
        data = load_data(dist, p)
        if data:
            plt.plot(num_modules, data, label=f'{p} procs')
            legend_labels.append(f'{p} procs')

            # Add saturation point if found
            sat_point = find_saturation_point(data)
            if sat_point:
                plt.axvline(x=sat_point, color='red', linestyle=':', linewidth=1)
                plt.text(sat_point + 3, data[sat_point], f'Sat ~{sat_point}', color='red', fontsize=8)

    # Add vertical reference lines
    for x in vlines:
        plt.axvline(x=x, color='gray', linestyle='--', linewidth=0.8)
        plt.text(x + 2, 1.05, f'{x}', rotation=90, fontsize=8, color='gray', verticalalignment='bottom')

    plt.xlabel('Memory Modules')
    plt.ylabel('Avg Access Time $T_c(S)$')
    plt.xscale('linear')
    plt.yscale('log')
    plt.title(f'{dist.capitalize()} Distribution')
    if legend_labels:
        plt.legend()
    plt.grid(True, which='both', linestyle='--', linewidth=0.5)
    plt.tight_layout()
    plt.savefig(f'{dist}_chart_saturation.png')
    print(f"Saved: {dist}_chart_saturation.png")
    plt.close()

# Run for both distributions
for dist in distributions:
    plot_distribution(dist)
