#!/bin/bash

set -eu

command="clang-format"
version=11

###

testing=0
cleanup=0

for option in "$@"; do
	[[ "$option" =~ ^--mode=testing$ ]] && testing=1
	[[ "$option" =~ ^--mode=cleanup$ ]] && cleanup=1
done

###

if [[ $version -gt 0 ]]; then
	command="${command}-${version}"
fi

# awk 'idx=index($0,"// clang-format off") {print idx}' $from

clang_format()
{
	if [[ $# -ne 1 ]]; then
		echo "Usage: $0 <file>"
		exit 1
	fi

	local readonly from="$1"
	local readonly from_format="$from.clang-format"

	local filename="${from%.*}"   # last dot
	local extension="${from##*.}" # last dot

	if [[ $extension == "cpp" ]]; then
		extension="clang-format.cxx"
	elif [[ $extension == "hpp" ]]; then
		extension="clang-format.hxx"
	else
		echo "Unknown extension '$extension'"
		exit 1
	fi

	local to="$filename.$extension"

	###

	if [[ "$filename" == "$to" ]]; then
		echo "assert : filename == to : '$filename' == '$to'"
		exit 1
	fi

	###

	if [[ $testing -eq 0 ]]; then
		"$command" -style=file --verbose -i "$from"
	else
		cp "$from" "$to"
		"$command" -style=file --verbose -i "$to"
	fi

	if [[ $cleanup -eq 1 ]]; then
		rm -f $to
	fi

}

###

for from in *.cpp *.hpp ssl/*.cpp ssl/*.hpp executable/*.cpp; do
	clang_format "$from"
done
