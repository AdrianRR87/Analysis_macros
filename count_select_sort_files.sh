#!/bin/bash

# Current directory containing the files
DIR="."  

# Parent directory's name (module ID)
module_id=$(basename "$(dirname "$PWD")")

# Last but one character of the module ID
sorting_char="${module_id: -2:1}"

# Determine the sorting pattern based on the last but one character
if [[ "$sorting_char" == "B" ]]; then
    # If "B", use electrons decreasing and holes pattern: 1,0,3,2,5,4,7,6
    elect_sort_order="decreasing"
    holes_sort_order=("HW_1" "HW_0" "HW_3" "HW_2" "HW_5" "HW_4" "HW_7" "HW_6")
elif [[ "$sorting_char" == "A" ]]; then
    # If "A", use holes decreasing and electrons pattern: 1,0,3,2,5,4,7,6
    elect_sort_order=("HW_1" "HW_0" "HW_3" "HW_2" "HW_5" "HW_4" "HW_7" "HW_6")
    holes_sort_order="decreasing"
else
    echo "Unknown sorting pattern for character '$sorting_char'."
    exit 1
fi

# Filter only files ending with elect.txt or holes.txt
FILES=($(find "$DIR" -maxdepth 1 -type f -name "*elect.txt" -o -name "*holes.txt"))

# Count the number of relevant files
FILE_COUNT=${#FILES[@]}

# Function to select the most recent file for each ASIC and polarity
select_most_recent() {
    local files=("$@")
    declare -A asic_files

    for file in "${files[@]}"; do
        basename=$(basename "$file")
        # Extract ASIC number (HW_X) and polarity (holes/elect.txt)
        asic=$(echo "$basename" | grep -oP 'HW_\d')
        polarity=$(echo "$basename" | grep -oP '(holes|elect)')
        
        # Extract date and time (assumed to be in the format `pscan_YYMMDD_HHMM`)
        date_time=$(echo "$basename" | grep -oP 'pscan_\K(\d{6}_\d{4})')

        # Combine ASIC, polarity, and date_time into a key
        key="${asic}_${polarity}"
        
        # Compare and store the most recent file
        if [[ -z "${asic_files[$key]}" ]] || [[ "$date_time" > "$(echo ${asic_files[$key]} | awk '{print $1}')" ]]; then
            asic_files[$key]="$date_time $file"
        fi
    done

    # Return the selected files
    for key in "${!asic_files[@]}"; do
        echo "${asic_files[$key]}" | awk '{print $2}'
    done
}

# Custom sorting function for holes and elect files
sort_files() {
    local files=("$@")
    holes_sorted=()
    elect_sorted=()

    for file in "${files[@]}"; do
        if [[ "$file" == *holes.txt ]]; then
            holes_sorted+=("$file")
        elif [[ "$file" == *elect.txt ]]; then
            elect_sorted+=("$file")
        fi
    done

    # Sort elect files based on the determined pattern
    if [[ "$elect_sort_order" == "decreasing" ]]; then
        elect_sorted=($(printf "%s\n" "${elect_sorted[@]}" | sort -t'_' -k6,6r))
    else
        sorted_elect=()
        for hw in "${elect_sort_order[@]}"; do
            for file in "${elect_sorted[@]}"; do    
                if [[ "$file" == *"$hw"* ]]; then
                    sorted_elect+=("$file")
                fi
            done
        done
        elect_sorted=("${sorted_elect[@]}")
    fi

    # Sort holes files based on the determined pattern
    if [[ "$holes_sort_order" == "decreasing" ]]; then
        holes_sorted=($(printf "%s\n" "${holes_sorted[@]}" | sort -t'_' -k6,6r))
    else
        sorted_holes=()
        for hw in "${holes_sort_order[@]}"; do
            for file in "${holes_sorted[@]}"; do
                if [[ "$file" == *"$hw"* ]]; then
                    sorted_holes+=("$file")
                fi
            done
        done
        holes_sorted=("${sorted_holes[@]}")
    fi

    # Print sorted files: elect first, then holes
    for file in "${elect_sorted[@]}"; do
        echo "$file"
    done
    for file in "${holes_sorted[@]}"; do
        echo "$file"
    done
}

if [[ $FILE_COUNT -lt 16 ]]; then
    echo "Warning: Less than 16 relevant files found."
    ./find_ASICs.sh
    SELECTED_FILES=("${FILES[@]}")
elif [[ $FILE_COUNT -eq 16 ]]; then
    SELECTED_FILES=("${FILES[@]}")
else
    echo "More than 16 files found. Selecting the most recent 16 files."

    # Separate files by polarity
    holes_files=()
    elect_files=()
    for file in "${FILES[@]}"; do
        if [[ "$file" == *holes.txt ]]; then
            holes_files+=("$file")
        elif [[ "$file" == *elect.txt ]]; then
            elect_files+=("$file")
        fi
    done

    # Select the most recent 8 holes files and 8 elect files
    selected_holes=$(select_most_recent "${holes_files[@]}" | sort | tail -n 8)
    selected_elect=$(select_most_recent "${elect_files[@]}" | sort | tail -n 8)

    # Combine selected files
    SELECTED_FILES=($selected_holes $selected_elect)
fi

# Sort the selected files
sorted_files=($(sort_files "${SELECTED_FILES[@]}"))

# Output the sorted and selected files
if [ -f a.txt ]; then
    rm a.txt
fi

echo "Selected files:"
for file in "${sorted_files[@]}"; do
    echo "$(basename "$file")" 
    echo "$(basename "$file")" >> a.txt
done

