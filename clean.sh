#!/bin/sh

find build/ -depth -mindepth 1 \( ! -name ".git_ignore" \) -print -delete

rm -Rf CMakeFiles

find -depth \( \( -name "*.o" -o -name "*.ppu" -o -name "*.rst" -o -name "*.aux" -o -name "*.snm" -o -name "*.out" -o -name "*.toc" -o -name "*.nav" -o -name "*.log" -o -name "*.backup" -o -name "*.bbl" -o -name "*.blg" -o -name "*~" -o -name "CMakeCache.txt" \) -a \! \( -wholename "*.git/*" -o -wholename "*.svn" \) \) -delete -print
