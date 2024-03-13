#!/bin/sh
for i in $(seq 1 100); do
    python3 judge.py 2>/dev/null
done
