# Write the platform descision code
IF (UNIX)
  SET(__PLATFORM PLATFORM_LINUX)
ELSE (UNIX)
  SET(__PLATFORM PLATFORM_WINDOWS)
ENDIF (UNIX)
CONFIGURE_FILE(config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

# Add the acinerella library
ADD_LIBRARY(acinerella SHARED acinerella.c)
TARGET_LINK_LIBRARIES(acinerella avformat avcodec avutil swscale pthread z m)

# Add the test application
ADD_EXECUTABLE(test test.c)
TARGET_LINK_LIBRARIES(test ${CMAKE_CURRENT_BINARY_DIR}/libacinerella.so)

