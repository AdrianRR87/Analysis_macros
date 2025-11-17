#!/bin/bash

# Function that processes each module directory
conn_check_gen() {
    folders=("$@")

    for module_id in "${folders[@]}"; do
        echo ">>> Processing module: $module_id"

        # --- Check if module_id is provided ---
        if [ -z "$module_id" ]; then
            echo "Error: module_id not provided."
            exit 1
        fi

        # --- Navigate to the module's pscan_files directory ---
        if [ ! -d "$module_id/pscan_files" ]; then
            echo "Warning: Directory '$module_id/conn_check_files' not found. Skipping."
            continue
        fi

        cd "$module_id/conn_check_files/" || { echo "Cannot access $module_id/conn_check_files"; continue; }

        # --- List all pscan_files*.txt larger than 1 KB ---
        find . -type f -name "conn_check_2*.txt" -size +1k | sed 's|.*/||' > a.txt

        # --- Copy necessary macro and helper files ---
        cp ../../../20251112/analysis_conn_check.C .

        # --- Clean old ROOT files ---
        rm -f *.root

        # --- Run ROOT reconstruction macros ---
        root -l -b <<EOF
.x analysis_conn_check.C
.q
EOF


        # --- Clean up temporary files ---
        rm -f analysis_conn_check.C
	rm -f a.txt
        echo ">>> Finished processing module: $module_id"
        echo "--------------------------------------------"

        # Go back to root directory for next iteration
        cd - > /dev/null || exit
    done
}

# --- Step 1: List all folders in the current directory ---
#mapfile -t folder_list < <(find . -mindepth 1 -maxdepth 1 -type d | sed 's|^\./||')
folder_list=()
while IFS= read -r d; do
    folder_list+=( "$d" )
done < <(find . -mindepth 1 -maxdepth 1 -type d | sed 's|^\./||')

# --- Step 2: Pass them to pscan_gen ---
conn_check_gen "${folder_list[@]}"
