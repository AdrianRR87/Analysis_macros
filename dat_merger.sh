#!/bin/bash

# Collect all folders in the current directory
folder_list=()
while IFS= read -r d; do
    folder_list+=( "$d" )
done < <(find . -mindepth 1 -maxdepth 1 -type d | sed 's|^\./||')

# Loop through every folder
for module in "${folder_list[@]}"; do
    echo "--------------------------------------"
    echo "Processing folder: $module"

    cd "$module" || { echo "Cannot enter directory $module"; continue; }

    # Enable expansion for dat files
    shopt -s nullglob
    dat_files=( *.dat )

    if [ ${#dat_files[@]} -eq 0 ]; then
        echo "No .dat files found in: $module"
        # still delete logs even if no dat files
        rm -f *.log
        cd - > /dev/null
        continue
    fi

    # First file becomes the merged base file
    base_file="${dat_files[0]}"
    echo "Base file (kept): $base_file"
    echo "Files to merge and then delete:"
    printf '  %s\n' "${dat_files[@]:1}"

    # Append all other dat files to the base file, separated by two blank lines
    for f in "${dat_files[@]:1}"; do
        # Add two blank lines for separation
        echo -e "\n" >> "$base_file"
        cat "$f" >> "$base_file"
    done

    # Delete all other dat files except the base file
    for f in "${dat_files[@]:1}"; do
        rm -f "$f"
    done

    # Delete all .log files in the folder
    rm -f *.log
    echo "Removed all .log files."

    echo "Merged and cleaned. Remaining file: $base_file"

    # Go back to the root directory
    cd - > /dev/null
done

echo "--------------------------------------"
echo "All folders processed."