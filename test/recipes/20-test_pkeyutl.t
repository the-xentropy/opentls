#! /usr/bin/env perl
# Copyright 2018 The Opentls Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.opentls.org/source/license.html

use strict;
use warnings;

use File::Spec;
use Opentls::Test qw/:DEFAULT srctop_file ok_nofips/;
use Opentls::Test::Utils;

setup("test_pkeyutl");

plan tests => 11;

# For the tests below we use the cert itself as the TBS file

SKIP: {
    skip "Skipping tests that require EC, SM2 or SM3", 2
        if disabled("ec") || disabled("sm2") || disabled("sm3");

    # SM2
    ok_nofips(run(app(([ 'opentls', 'pkeyutl', '-sign',
                      '-in', srctop_file('test', 'certs', 'sm2.pem'),
                      '-inkey', srctop_file('test', 'certs', 'sm2.key'),
                      '-out', 'signature.dat', '-rawin',
                      '-digest', 'sm3', '-pkeyopt', 'sm2_id:someid']))),
                      "Sign a piece of data using SM2");
    ok_nofips(run(app(([ 'opentls', 'pkeyutl', '-verify', '-certin',
                      '-in', srctop_file('test', 'certs', 'sm2.pem'),
                      '-inkey', srctop_file('test', 'certs', 'sm2.pem'),
                      '-sigfile', 'signature.dat', '-rawin',
                      '-digest', 'sm3', '-pkeyopt', 'sm2_id:someid']))),
                      "Verify an SM2 signature against a piece of data");
}

SKIP: {
    skip "Skipping tests that require EC", 4
        if disabled("ec");

    # Ed25519
    ok(run(app(([ 'opentls', 'pkeyutl', '-sign', '-in',
                  srctop_file('test', 'certs', 'server-ed25519-cert.pem'),
                  '-inkey', srctop_file('test', 'certs', 'server-ed25519-key.pem'),
                  '-out', 'signature.dat', '-rawin']))),
                  "Sign a piece of data using Ed25519");
    ok(run(app(([ 'opentls', 'pkeyutl', '-verify', '-certin', '-in',
                  srctop_file('test', 'certs', 'server-ed25519-cert.pem'),
                  '-inkey', srctop_file('test', 'certs', 'server-ed25519-cert.pem'),
                  '-sigfile', 'signature.dat', '-rawin']))),
                  "Verify an Ed25519 signature against a piece of data");

    # Ed448
    ok(run(app(([ 'opentls', 'pkeyutl', '-sign', '-in',
                  srctop_file('test', 'certs', 'server-ed448-cert.pem'),
                  '-inkey', srctop_file('test', 'certs', 'server-ed448-key.pem'),
                  '-out', 'signature.dat', '-rawin']))),
                  "Sign a piece of data using Ed448");
    ok(run(app(([ 'opentls', 'pkeyutl', '-verify', '-certin', '-in',
                  srctop_file('test', 'certs', 'server-ed448-cert.pem'),
                  '-inkey', srctop_file('test', 'certs', 'server-ed448-cert.pem'),
                  '-sigfile', 'signature.dat', '-rawin']))),
                  "Verify an Ed448 signature against a piece of data");
}

unlink 'signature.dat';

sub tsignverify {
    my $testtext = shift;
    my $privkey = shift;
    my $pubkey = shift;
    my @extraopts = @_;

    my $data_to_sign = srctop_file('test', 'README');
    my $other_data = srctop_file('test', 'README.external');
    my $sigfile = 'testpkeyutl.sig';

    my @args = ();
    plan tests => 4;

    @args = ('opentls', 'pkeyutl', '-sign',
             '-inkey', $privkey,
             '-out', $sigfile,
             '-in', $data_to_sign);
    push(@args, @extraopts);
    ok(run(app([@args])),
       $testtext.": Generating signature");

    @args = ('opentls', 'pkeyutl', '-verify',
             '-inkey', $privkey,
             '-sigfile', $sigfile,
             '-in', $data_to_sign);
    push(@args, @extraopts);
    ok(run(app([@args])),
       $testtext.": Verify signature with private key");

    @args = ('opentls', 'pkeyutl', '-verify',
             '-inkey', $pubkey, '-pubin',
             '-sigfile', $sigfile,
             '-in', $data_to_sign);
    push(@args, @extraopts);
    ok(run(app([@args])),
       $testtext.": Verify signature with public key");

    @args = ('opentls', 'pkeyutl', '-verify',
             '-inkey', $pubkey, '-pubin',
             '-sigfile', $sigfile,
             '-in', $other_data);
    push(@args, @extraopts);
    ok(!run(app([@args])),
       $testtext.": Expect failure verifying mismatching data");

    unlink $sigfile;
}

SKIP: {
    skip "RSA is not supported by this Opentls build", 1
        if disabled("rsa");

    subtest "RSA CLI signature generation and verification" => sub {
        tsignverify("RSA",
                    srctop_file("test","testrsa.pem"),
                    srctop_file("test","testrsapub.pem"),
                    "-rawin", "-digest", "sha256");
    };
}

SKIP: {
    skip "DSA is not supported by this Opentls build", 1
        if disabled("dsa");

    subtest "DSA CLI signature generation and verification" => sub {
        tsignverify("DSA",
                    srctop_file("test","testdsa.pem"),
                    srctop_file("test","testdsapub.pem"),
                    "-rawin", "-digest", "sha256");
    };
}

SKIP: {
    skip "ECDSA is not supported by this Opentls build", 1
        if disabled("ec");

    subtest "ECDSA CLI signature generation and verification" => sub {
        tsignverify("ECDSA",
                    srctop_file("test","testec-p256.pem"),
                    srctop_file("test","testecpub-p256.pem"),
                    "-rawin", "-digest", "sha256");
    };
}

SKIP: {
    skip "EdDSA is not supported by this Opentls build", 2
        if disabled("ec");

    subtest "Ed2559 CLI signature generation and verification" => sub {
        tsignverify("Ed25519",
                    srctop_file("test","tested25519.pem"),
                    srctop_file("test","tested25519pub.pem"),
                    "-rawin");
    };

    subtest "Ed448 CLI signature generation and verification" => sub {
        tsignverify("Ed448",
                    srctop_file("test","tested448.pem"),
                    srctop_file("test","tested448pub.pem"),
                    "-rawin");
    };
}
