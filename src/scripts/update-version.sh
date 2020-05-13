#!/bin/bash

basedir=$(dirname $0)

version=$(cat $basedir/../../version)

for f in $(find $basedir/.. -name 'Info.plist.in' -o -name 'version.hpp.in'); do
    outfile=$(dirname $f)/$(basename $f .in)
    sed "s|@VERSION@|$version|g" $f >$outfile.tmp
    if cmp -s $outfile.tmp $outfile; then
        echo "Skip $outfile"
        rm $outfile.tmp
    else
        echo "Update $outfile"
        mv $outfile.tmp $outfile
    fi
done
