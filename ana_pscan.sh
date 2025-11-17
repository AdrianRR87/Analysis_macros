#!/bin/bash

# Check if module_id is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <module_id>"
  exit 1
fi

module_id=$(basename "$1")


# Navigate to the module's pscan_files directory
cd "$module_id/pscan_files/" || exit
find . -type f -name "pscan_files*.txt" -size +1k | sed 's|.*/||' > a.txt

# Copy the necessary files from the source directory
cp ../../../../analysis_macros/20240419/execution.C .
cp ../../../../analysis_macros/20240419/trim_adc.cxx .
cp ../../../../analysis_macros/20240419/trim_adc.hxx .
cp ../../../../analysis_macros/20240419/plot_1024_MT.C .

# List the relevant pscan files and save to a.txt
cp ../../find_ASICs.sh .
cp ../../count_select_sort_files.sh .
./count_select_sort_files.sh

# Run ROOT commands
root -l -b <<EOF
.L trim_adc.cxx
.L execution.C
execution()
.q
EOF

# List the generated ROOT files and save to plot.txt
cp ../../count_select_sort_root_files.sh .
./count_select_sort_root_files.sh

# Run the plotting macro
root -l -b <<EOF
.x plot_1024_MT.C
.q
EOF

# Removing unnecessary files
rm trim_adc.cxx
rm trim_adc.hxx
rm execution.C
