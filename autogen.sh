#!/bin/sh

echo
echo ... Freetalk autogen ...
echo

## Check all dependencies are present
MISSING=""

# Check for aclocal
env aclocal --version > /dev/null 2>&1
if [ $? -eq 0 ]; then
    ACLOCAL=aclocal
else
    MISSING="$MISSING aclocal"
fi

# Check for autoconf
env autoconf --version > /dev/null 2>&1
if [ $? -eq 0 ]; then
    AUTOCONF=autoconf
else
    MISSING="$MISSING autoconf"
fi

# Check for autoheader
env autoheader --version > /dev/null 2>&1
if [ $? -eq 0 ]; then
    AUTOHEADER=autoheader
else
    MISSING="$MISSING autoheader"
fi

# Check for automake
env automake --version > /dev/null 2>&1
if [ $? -eq 0 ]; then
    AUTOMAKE=automake
else
    MISSING="$MISSING automake"
fi

# Check for tar
env tar -cf /dev/null /dev/null > /dev/null 2>&1
if [ $? -ne 0 ]; then
    MISSING="$MISSING tar"
fi

## If dependencies are missing, warn the user and abort
if [ "x$MISSING" != "x" ]; then
    echo "Aborting."
    echo
    echo "The following build tools are missing:"
    echo
    for pkg in $MISSING; do
        echo "  * $pkg"
    done
    echo
    echo "Please install them and try again."
    echo
    exit 1
fi

## Do the autogeneration
echo Running ${ACLOCAL}...
$ACLOCAL
echo Running ${AUTOHEADER}...
$AUTOHEADER
echo Running ${AUTOCONF}...
$AUTOCONF
echo Running ${AUTOMAKE}...
$AUTOMAKE --add-missing --copy --foreign

# Run autogen in the argp-standalone sub-directory
echo "Running autogen.sh in argp-standalone ..."
( cd contrib/argp-standalone;./autogen.sh )

# Instruct user on next steps
echo
echo "Please proceed with configuring, compiling, and installing."
