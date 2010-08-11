#/bin/sh
find . -name "backup" -exec rm -R {} \;
find . -name "CMakeFiles" -exec rm -R {} \;
find . -name "Makefile" -exec rm -R {} \;
find . -name "*.ppu" -exec rm -R {} \;
find . -name "*.ppm" -exec rm -R {} \;
find . -name "*.raw" -exec rm -R {} \;
find . -name "*.wav" -exec rm -R {} \;
find . -name "*.o" -exec rm -R {} \;
find . -name "cmake_install.cmake" -exec rm -R {} \;

rm -f src/config.h

