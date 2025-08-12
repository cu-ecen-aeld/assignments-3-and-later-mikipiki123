#!/bin/bash

if [[ -z "$1" || -z "$2" ]]; then
	echo "arguments not specified."
	exit 1
fi

writefile="$1"
writestr="$2"

echo "PATH: $writefile"

abs_path=$(realpath -m "$writefile")
echo "ABS: $abs_path"

dirpath=$(dirname "$abs_path")

echo "DIR: $dirpath"
mkdir -p "$dirpath"


echo "$writestr" > "$writefile"

if [ ! -f "$writefile" ]; then
        echo " file isn't created."
        exit 1
fi


