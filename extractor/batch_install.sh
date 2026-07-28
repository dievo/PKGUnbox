#!/bin/bash

scriptDir="$(dirname "$0")"

if [ -z "$1" ] || [ ! -d "$1" ]; then
    echo "Error: Please provide a valid directory containing .pkg files."
    echo "Usage: $0 /path/to/pkg/directory"
    read -r
    exit 1
fi

oldIFS="$IFS"
IFS=$'\n'

count=0
for f in "$1"/*.pkg; do
    [ -e "$f" ] || continue
    count=$((count + 1))
    "$scriptDir/install_pkg.sh" "$f" --batch
done

IFS="$oldIFS"

if [ "$count" -eq 0 ]; then
    echo "No .pkg files found in $1"
else
    echo "Batch install done ($count file(s) processed)."
fi

read -r
