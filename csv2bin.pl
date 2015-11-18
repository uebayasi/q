#! /usr/bin/env perl
use strict;
my (@fmts);
my $line = <>;
@fmts = split(/\s*,\s*/, $line);
while (my $line = <>) {
	my @nums = split(/\s*,\s*/, $line);
	my $i = 0;
	foreach my $fmt (@fmts) {
		print(pack($fmt, $nums[$i]));
		$i++;
	}
}
