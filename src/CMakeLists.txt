# Set the FFMpeg include path
INCLUDE_DIRECTORIES(${FFMPEG_INCLUDE_DIR})

SET(CMAKE_C_STANDARD 11)

# Enable all warnings on MSVC and GCC/Clang/Intel
if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
	if(CMAKE_C_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
	else()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
	endif()
elseif(("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU") OR
		("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang") OR
		("${CMAKE_C_COMPILER_ID}" STREQUAL "Intel"))
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")
endif()

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
