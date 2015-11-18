#! /usr/bin/env perl

use strict;

my ($tab, $tabs);
my ($qry, $qrys);
my ($pad);
my $type2c = {
	"I" => "int\%d_t",
	"U" => "uint\%d_t",
};

sub parse_table {
	my ($name) = @_;
	if (defined($tab)) {
		push @{$tabs}, $tab;
		undef $tab;
	}
	$tab->{name} = $name;
}

sub parse_column {
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

sub parse_qry {
	my ($name) = @_;
	if (defined($qry)) {
		push @{$qrys}, $qry;
		undef $qry;
	}
	$qry->{name} = $name;
}

sub parse {
	while (my $line = <>) {
		if ($line =~ m/^TABLE\s+(.*?)$/) {
			parse_table($1);
		}
		if ($line =~ m/^COLUMN\s+(.*?)\s+(.*?)$/) {
			parse_column($1, $2);
		}
		if ($line =~ m/^QUERY\s+(.*?)$/) {
			parse_qry($1);
		}
	}
}

sub print_tab_col {
	my ($col) = @_;
	my ($type, $name, $array);
	if ($col->{size}) {
		$type = sprintf $type2c->{$col->{type}}, $col->{size};
	} else {
		$type = $type2c->{$col->{type}};
	}
	if ($col->{array}) {
		$array = sprintf "[%s]", $col->{array};
	} else {
		$array = "";
	}
	if ($col->{name}) {
		$name = $col->{name};
	} else {
		$name = sprintf "__pad%d", $pad++;
	}
	printf "\t%s %s%s;\n", $type, $name, $array;
}

sub print_qry_col {
	# XXX
}

sub print_tab {
	my ($tab) = @_;
	$pad = 1;
	printf "struct %s {\n", $tab->{name};
	foreach my $col (@{$tab->{cols}}) {
		print_tab_col($col);
	}
	printf "};\n";
}

sub print_qry {
	my ($qry) = @_;
	printf "struct %s {\n", $qry->{name};
	foreach my $col (@{$qry->{cols}}) {
		print_qry_col($col);
	}
	printf "};\n";
}

sub main {
	parse();
	if (defined($tab)) {
		push @{$tabs}, $tab;
		undef $tab;
	}
	if (defined($qry)) {
		push @{$qrys}, $qry;
		undef $qry;
	}
	# XXX do checks
	foreach (@{$tabs}) {
		print_tab($_);
	}
	foreach (@{$qrys}) {
		print_qry($_);
	}
}

main();
