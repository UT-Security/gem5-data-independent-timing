#!/bin/bash

max_jobs=12
job_count=0

echo "Cleaning existing output directories..."

# Extract output directories from commands and clean them
while IFS= read -r cmd; do
    if [[ $cmd =~ --outdir=([^[:space:]]+) ]]; then
        outdir="${BASH_REMATCH[1]}"
        # Check if we're in a polybench directory and clean the outdir there
        if [[ $cmd =~ ^cd[[:space:]]+([^[:space:]]+) ]]; then
            subdir="${BASH_REMATCH[1]%/}"  # Remove trailing slash if present
            if [ -d "$subdir/$outdir" ]; then
                echo "Cleaning $subdir/$outdir"
                rm -rf "$subdir/$outdir"
            fi
        fi
    fi
done < polybench_commands.txt

echo "Starting benchmark runs..."

while IFS= read -r cmd; do
    eval "$cmd" &

    ((job_count++))
    if (( job_count % max_jobs == 0 )); then
        wait  # wait for all background jobs
    fi
done < polybench_commands.txt
