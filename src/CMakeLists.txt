# Acinerella -- ffmpeg Wrapper Library
# Copyright (C) 2008-2018  Andreas Stöckel
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with Acinerella.  If not, see <http://www.gnu.org/licenses/>.

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
  TARGET_LINK_LIBRARIES(acinerella ${FFMPEG_LIBRARIES} pthread z m)
ELSE (UNIX)
  TARGET_LINK_LIBRARIES(acinerella ${FFMPEG_LIBRARIES})
ENDIF (UNIX)

# Add the test application
ADD_EXECUTABLE(demo demo.c)
TARGET_LINK_LIBRARIES(demo acinerella)
INSTALL(TARGETS acinerella LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
