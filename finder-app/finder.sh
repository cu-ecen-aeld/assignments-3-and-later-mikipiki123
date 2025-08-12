#!/bin/bash

if [[ -z "$1" || -z "$2" ]]; then
	echo "one of the arguments aren't specified."
	exit 1
fi

if [ ! -d "$1" ]; then
	echo "there's no such directory."
	exit 1
fi

filesdir="$1"
searchstr="$2"

x=$(find "$filesdir" -type f | wc -l)

y=$(grep -rF "$searchstr" "$filesdir" | wc -l)

echo "The number of files are $x and the number of matching lines are $y"

