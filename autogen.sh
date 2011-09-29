#!/bin/bash
autopoint -f
aclocal
autoconf
automake --add-missing

echo "Now run ./configure and make"
