#!/usr/bin/perl

# Copyright (C) Nginx, Inc.

# Tests for hello_world module.

###############################################################################

use warnings;
use strict;

use Test::More;
use Test::Nginx;

###############################################################################

select STDERR; $| = 1;
select STDOUT; $| = 1;

my $t = Test::Nginx->new()->has(qw/http hello_world/)->plan(2);

$t->write_file_expand('nginx.conf', <<'EOF');

%%TEST_GLOBALS%%

daemon off;

events {
}

http {
    %%TEST_GLOBALS_HTTP%%

    server {
        listen       127.0.0.1:8080;
        server_name  localhost;

        location / {
            hello_world;
            hello_world_text "Hello, universe!";
            hello_world_status 201;
        }
    }
}

EOF

$t->run();

###############################################################################

like(http_get('/'), qr/Hello, universe!/, 'get hello world text');
like(http_get('/'), qr/201 Created/i, 'get response code');

###############################################################################
