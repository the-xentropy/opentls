#! /usr/bin/env perl
# Copyright 2015-2016 The Opentls Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.opentls.org/source/license.html


use Opentls::Test qw/:DEFAULT srctop_file srctop_dir/;
use Opentls::Test::Simple;

setup("test_ct");
$ENV{CTLOG_FILE} = srctop_file("test", "ct", "log_list.conf");
$ENV{CT_DIR} = srctop_dir("test", "ct");
$ENV{CERTS_DIR} = srctop_dir("test", "certs");
simple_test("test_ct", "ct_test", "ct", "ec");
