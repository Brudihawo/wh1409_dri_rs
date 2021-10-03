#!/usr/bin/bash

# process all input signals in ROOT/data/in and write processed signals to
# ROOT/data/out

cur_path=$0
in_path=$(realpath "$(dirname $cur_path/)/../data/in")
out_path=$(realpath "$(dirname $cur_path/)/../data/out")

files=$(ls $in_path)

for file in ${files[@]}; do
  echo "Processing $in_path/$file..."
  if [[ -f "$in_path/$file" ]]; then
    echo "test"
    cargo run -- $in_path/$file > $out_path/$(echo $file | sed 's/.txt/.tsv/')
  fi
done
