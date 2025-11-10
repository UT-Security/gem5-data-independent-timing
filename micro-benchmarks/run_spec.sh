#!/bin/bash

max_jobs=12
job_count=0

while IFS= read -r cmd; do
    eval "$cmd" &

    ((job_count++))
    if (( job_count % max_jobs == 0 )); then
        wait  # wait for all background jobs
    fi
done < spec_commands.txt
