my ($tab, $tabs);
my ($q, $qs);
sub parse_table {
}
sub parse_column {
}
sub parse_query {
}
sub parse {
}
while (my $line = <>) {
	if ($line =~ m/^TABLE\s+(.*?)$/) {
		if (defined($tab)) {
			push @{$tabs}, $tab;
			undef $tab;
		}
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
	if ($line =~ m/^QUERY\s+(.*?)$/) {
		if (defined($q)) {
			push @{$qs}, $q;
			undef $q;
		}
		$q->{name} = $1;
	}
}
if (defined($tab)) {
	push @{$tabs}, $tab;
	undef $tab;
}
if (defined($q)) {
	push @{$qs}, $q;
	undef $q;
}
# XXX do checks
my $type2c = {
	"I" => "int\%d_t ",
	"U" => "uint\d_t ",
	"PTR" => "void *",
};
sub print_tab_col {
	my ($col) = @_;
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
sub print_tab {
	my ($tab) = @_;
	my $pad = 1;
	printf "struct %s {\n", $tab->{name};
	foreach my $col (@{$tab->{cols}}) {
		print_tab_col($col);
	}
	printf "};\n";
}
sub print_query {
	my ($q) = @_;
	printf "struct %s {\n", $q->{name};
	printf "};\n";
}
foreach my $tab (@{$tabs}) {
	print_tab($tab);
}
foreach my $q (@{$qs}) {
	print_tab($q);
}
