#!/bin/bash

# Function that processes each module directory
pscan_gen() {
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
            echo "Warning: Directory '$module_id/pscan_files' not found. Skipping."
            continue
        fi

        cd "$module_id/pscan_files/" || { echo "Cannot access $module_id/pscan_files"; continue; }

        # --- List all pscan_files*.txt larger than 1 KB ---
        find . -type f -name "pscan_files*.txt" -size +1k | sed 's|.*/||' > a.txt

        # --- Copy necessary macro and helper files ---
        cp ../../../../analysis_macros/20251112/execution.C .
        cp ../../../../analysis_macros/20251112/trim_adc.cxx .
        cp ../../../../analysis_macros/20251112/trim_adc.hxx .
        cp ../../../../analysis_macros/20251112/plot_1024_MT.C .

        # --- Clean old ROOT files ---
        rm -f *.root

        # --- Prepare and run scripts for ASIC detection and file counting ---
        cp ../../../../analysis_macros/20251112/find_ASICs.sh .
        cp ../../../../analysis_macros/20251112/count_select_sort_files.sh .
        ./count_select_sort_files.sh

        # --- Run ROOT reconstruction macros ---
        root -l -b <<EOF
.L trim_adc.cxx
.L execution.C
execution()
.q
EOF

        # --- List generated ROOT files ---
        cp ../../../../analysis_macros/20251112/count_select_sort_root_files.sh .
        ./count_select_sort_root_files.sh

        # --- Run ROOT plotting macro ---
        root -l -b <<EOF
.x plot_1024_MT.C
.q
EOF

        # --- Clean up temporary files ---
        rm -f trim_adc.cxx trim_adc.hxx execution.C plot_1024_MT.C
        rm -f find_ASICs.sh count_select_sort_files.sh count_select_sort_root_files.sh
	rm -f a.txt plot.txt
        echo ">>> Finished processing module: $module_id"
        echo "--------------------------------------------"

        # Go back to root directory for next iteration
        cd - > /dev/null || exit
    done
}

# --- Step 1: List all folders in the current directory ---
mapfile -t folder_list < <(find . -mindepth 1 -maxdepth 1 -type d | sed 's|^\./||')

# --- Step 2: Pass them to pscan_gen ---
pscan_gen "${folder_list[@]}"
