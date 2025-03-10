#!/bin/bash


for var in "${!SLURM_@}"; do
    printf '%s=%s\n' "$var" "${!var}"
done

