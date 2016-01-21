# Set the FFMpeg include path
INCLUDE_DIRECTORIES(${FFMPEG_INCLUDE_DIR})

# Write the platform decision code
IF(UNIX)
  SET(__PLATFORM PLATFORM_LINUX)
ELSE()
  SET(__PLATFORM PLATFORM_WINDOWS)
ENDIF()
CONFIGURE_FILE(config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

# Add the acinerella library
ADD_LIBRARY(acinerella SHARED acinerella.c acinerella.h)

IF (UNIX)
  TARGET_LINK_LIBRARIES(acinerella avformat avcodec avutil swscale pthread z m)
ELSE (UNIX)
  TARGET_LINK_LIBRARIES(acinerella avformat avcodec avutil swscale pthread z m ws2_32)
ENDIF (UNIX)

# Add the test application
ADD_EXECUTABLE(demo demo.c)
TARGET_LINK_LIBRARIES(demo acinerella)
INSTALL(TARGETS acinerella LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
