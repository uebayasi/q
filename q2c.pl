my ($tab);
my ($line);
while ($line = <>) {
	if ($line =~ m/^TABLE\s+(.*?)$/) {
		$tab->{name} = $1;
	}
	if ($line =~ m/^COLUMN\s+(.*?)\s+(.*?)$/) {
		my ($type, $name) = ($1, $2);
		my ($array);
		print STDERR "type: ", $type, "\n";
		print STDERR "name: ", $name, "\n";
		if ($type =~ m/^(.*?)\((\d+?)\)$/) {
			$type = $1;
			$array = $2;
		} else {
			undef($array);
		}
		push @{$tab->{cols}}, {
			"type" => $type,
			"name" => $name,
			"array" => $array,
		};
	}
}
# XXX do checks
my $type2c = {
	"I8" => "int8_t ",
	"U8" => "uint8_t ",
	"I16" => "int16_t ",
	"U16" => "uint16_t ",
	"I32" => "int32_t ",
	"U32" => "uint32_t ",
	"I64" => "int64_t ",
	"U64" => "uint64_t ",
	"PTR" => "void *",
};
printf "struct %s {\n", $tab->{name};
my $pad = 1;
foreach my $col (@{$tab->{cols}}) {
	if (not $col->{name}) {
		$col->{name} = sprintf "__pad%d", $pad++;
	}
	if ($col->{array}) {
		printf "\t%s%s[%s];\n", $type2c->{$col->{type}}, $col->{name}, $col->{array};
	} else {
		printf "\t%s%s;\n", $type2c->{$col->{type}}, $col->{name};
	}
}
printf "};\n";
