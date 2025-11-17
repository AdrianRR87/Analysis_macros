#!/bin/bash

# Directory containing the files
DIR="."  

# Get parent directory name to determine module ID
PARENT_DIR=$(basename "$(dirname "$PWD")")
LAST_BUT_ONE_CHAR=${PARENT_DIR: -2:1}

# Filter only files ending with elect.txt or holes.txt
FILES=($(find "$DIR" -maxdepth 1 -type f -name "*elect.txt" -o -name "*holes.txt"))

# ASIC names array for reference (HW_0 to HW_7)
ASIC_NAMES=("HW_0" "HW_1" "HW_2" "HW_3" "HW_4" "HW_5" "HW_6" "HW_7")

# Initialize arrays to assume all ASICs are present
ena_asics_h=(1 1 1 1 1 1 1 1)
ena_asics_e=(1 1 1 1 1 1 1 1)

# Process the files to check which ASICs are missing
for asic in "${ASIC_NAMES[@]}"; do
    asic_found_h=0
    asic_found_e=0

    for file in "${FILES[@]}"; do
        if [[ "$file" == *"$asic"*holes.txt ]]; then
            asic_found_h=1
        elif [[ "$file" == *"$asic"*elect.txt ]]; then
            asic_found_e=1
        fi
    done

    index=${asic//HW_/}

    # Update ena_asics_h and ena_asics_e based on the presence of files
    if [[ $asic_found_h -eq 0 ]]; then
        ena_asics_h[$index]=0
    fi
    if [[ $asic_found_e -eq 0 ]]; then
        ena_asics_e[$index]=0
    fi
done

# Join array elements into a comma-separated string
ena_asics_h_str=$(IFS=,; echo "${ena_asics_h[*]}")
ena_asics_e_str=$(IFS=,; echo "${ena_asics_e[*]}")

# Modify the plot_1024_Maria.C script based on the final ena_asics arrays
echo "Modifying plot_1024_Maria.C script."
sed -i "/int ena_asics_h\[8\] =/c\int ena_asics_h[8] = {$ena_asics_h_str};" plot_1024_Maria.C
sed -i "/int ena_asics_e\[8\] =/c\int ena_asics_e[8] = {$ena_asics_e_str};" plot_1024_Maria.C

# Output the final ena_asics arrays for confirmation
echo "Final ena_asics_h: {$ena_asics_h_str}"
echo "Final ena_asics_e: {$ena_asics_e_str}"
