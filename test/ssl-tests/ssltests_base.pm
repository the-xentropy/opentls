# -*- mode: perl; -*-
# Copyright 2016 The Opentls Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.opentls.org/source/license.html

## tls test configurations

package tlstests;

sub test_pem
{
    my ($file) = @_;
    my $dir_sep = $^O ne "VMS" ? "/" : "";
    return "\${ENV::TEST_CERTS_DIR}" . $dir_sep . $file,
}

our %base_server = (
    "Certificate" => test_pem("servercert.pem"),
    "PrivateKey"  => test_pem("serverkey.pem"),
    "CipherString" => "DEFAULT",
);

our %base_client = (
    "VerifyCAFile" => test_pem("rootcert.pem"),
    "VerifyMode" => "Peer",
    "CipherString" => "DEFAULT",
);
