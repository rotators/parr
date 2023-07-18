#!/bin/bash

set -eu

command="clang-format"
version=11

###

testing=0
cleanup=0

for option in "$@"; do
	[[ "$option" =~ ^--debug$ ]] && set -x
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

	if [[ $testing -eq 0 ]]; then
		local output="$from"
	else
		cp "$from" "$to"
		local output="$to"
	fi

	"$command" -style=file --verbose -i "$output"

	###

	local edit='
	use strict;
	use warnings;

	my( $edit, $max, @lines ) = ( 0, 0 );
	while( <> )
	{
		chomp;
		my( $line, $editable ) = ( $_, 0 ) ;
		if( $line =~ /\/\/ clang-format-sh (on|off)/ )
		{
			my $mode = $1;
			$edit = 1 if( $mode eq "on" );
			$edit = 0 if( $mode eq "off" );
		}
		elsif( $edit && $line =~ /^[ ]*(struct|using).+[ ]+([:=])[ ]+/ )
		{
			my $char = $2;
			$line =~ s/[\t ]+([:=])[\t ]+/ $char /;
			$line =~ s/[\t ]*{[\t ]*}[\t ]*;/{};/;
			$line =~ s/>[\t ]+;/>;/;

			my $idx = index( $line, " $char " );
			if( $idx > $max )
			{
				$max = $idx;
			}
			$editable = 1;
		}
		elsif( $edit && scalar(@lines) && $lines[-1][1] == 1 && $line =~ /^[ ]*({};.*)$/ )
		{
			my $leftover = $1;
			$lines[-1][0] .= $leftover;
			next;
		}

		push( @lines, [ $line, $editable ] );
	};

	foreach my $data( @lines )
	{
		my $line = @{$data}[0];
		my $editable = @{$data}[1];

		if( $editable && $line =~ /^([ ]*)(struct|using)(.+)([ ][:=][ ])(.+)/ )
		{
			$line = sprintf( "%-" . $max ."s%s%s", $1.$2.$3, $4, $5 );
		}

		printf( "%s\n", $line );
	}'

	local can_edit=0
	grep --quiet "// clang-format-sh on" "$output" && can_edit=1 || can_edit=0

	if [[ $can_edit -eq 1 ]]; then
		perl -e "$edit" "$output" > "$output.changed"
		if cmp --silent "$output" "$output.changged"; then
			rm "$output.changed"
		else
			mv "$output.changed" "$output"
		fi
	fi

	if [[ $cleanup -eq 1 ]]; then
		rm -f $to
	fi

}

###

for from in *.cpp *.hpp ssl/*.cpp ssl/*.hpp executable/*.cpp; do
	clang_format "$from"
done
