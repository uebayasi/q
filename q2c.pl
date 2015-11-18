my ($tab);
my ($line);
while ($line = <>) {
	if ($line =~ m/^TABLE\s+(.*?)$/) {
		$tab->{name} = $1;
	}
	if ($line =~ m/^COLUMN\s+(.*?)\s+(.*?)$/) {
		my ($type, $name) = ($1, $2);
		my ($size, $array);
		print STDERR "type: ", $type, "\n";
		print STDERR "name: ", $name, "\n";
		if ($type =~ m/^(.*?)\((\d+?)\)$/) {
			$type = $1;
			$array = $2;
		} else {
			undef $array;
		}
		if ($type =~ m/^(.*?)(\d+?)$/) {
			$type = $1;
			$size = $2;
		} else {
			undef $size;
		}
		push @{$tab->{cols}}, {
			"type" => $type,
			"size" => $size,
			"name" => $name,
			"array" => $array,
		};
	}
}
# XXX do checks
my $type2c = {
	"I" => "int\%d_t ",
	"U" => "uint\d_t ",
	"PTR" => "void *",
};
printf "struct %s {\n", $tab->{name};
my $pad = 1;
foreach my $col (@{$tab->{cols}}) {
	my ($type, $name);
	if ($col->{size}) {
		$type = sprintf $type2c->{$col->{type}}, $col->{size};
	} else {
		$type = $type2c->{$col->{type}};
	}
	if ($col->{array}) {
		$name = sprintf "%s[%s]", $col->{name}, $col->{array};
	} else {
		$name = sprintf "%s", $col->{name};
	}
	if (not $name) {
		$name = sprintf "__pad%d", $pad++;
	}
	printf "\t%s%s;\n", $type, $name;
}
printf "};\n";
