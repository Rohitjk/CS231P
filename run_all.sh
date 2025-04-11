#!/bin/bash

# Make sure simulator.bin exists and is executable
if [[ ! -x ./simulator.bin ]]; then
  echo "Error: simulator.bin not found or not executable."
  exit 1
fi

# List of processor counts to test
processors=(2 4 8 16 32 64)

# Run simulations for each processor count and distribution
for dist in u n; do
  for p in "${processors[@]}"; do
    if [ "$dist" == "u" ]; then
      filename="uniform_${p}.csv"
    else
      filename="normal_${p}.csv"
    fi

    echo "Running: ./simulator.bin $p $dist > $filename"
    ./simulator.bin "$p" "$dist" > "$filename"
  done
done

echo "All simulations complete."
