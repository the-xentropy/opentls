#! /usr/bin/env perl
# Copyright 2019 The Opentls Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.opentls.org/source/license.html


use strict;
use warnings;

use Opentls::Test;

setup("test_provider");

plan tests => 7;

 SKIP: {
     skip "No default provider?", 6
         unless ok(run(app([qw(opentls provider default)])),
                   "try running 'opentls provider default'");

     my $prev = 2;              # The amount of lines from -v
     my @checks = qw( -v -vv -vvv );
     my %op = ( -v => '==',
                -vv => '>',
                -vvv => '>' );
     my $i = 0;

     foreach (@checks) {
         my @cmd = ('opentls', 'provider', $_, 'default');
         my @lines = ( map { (my $x = $_) =~ s|\R$||; $x }
                       run(app([@cmd]), capture => 1) );

         my $curr = scalar @lines;
         my $cmp = "$curr $op{$_} $prev";

         ok(eval $cmp,
            "'opentls provider $_ default' line count $op{$_} $prev");
         ok($lines[0] eq '[ default ]',
            "'opentls provider -v default' first line is '[ default ]'");

         $prev = $curr;
     }
}

