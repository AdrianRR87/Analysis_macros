#!/bin/bash

# Check if module_id is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <module_id>"
  exit 1
fi

module_id=$(basename "$1")

# Navigate to the module's conn_check_files directory
cd "$module_id/conn_check_files/" || exit
find . -type f -name "conn_check_2*.txt" -size +1k | sed 's|.*/||' > a.txt

# Copy the necessary files from the source directory
cp ../../../analysis_macros/20251112/analysis_conn_check.C .

# Run ROOT commands
root -l -b <<EOF
.x analysis_conn_check.C
.q
EOF
