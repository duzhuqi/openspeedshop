################################################################################
# Copyright (c) 2014-2016 Krell Institute. All Rights Reserved.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA
################################################################################

set(RT_SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/blobs.h
        ${CMAKE_CURRENT_BINARY_DIR}/blobs.c
	runtime.c
    )
set(PLUGIN_SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/blobs.h
        ${CMAKE_CURRENT_BINARY_DIR}/blobs.c
	UserTimeDetail.hxx
        UserTimeCollector.hxx UserTimeCollector.cxx
    )

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/blobs.h
        ${CMAKE_CURRENT_BINARY_DIR}/blobs.c
    COMMAND rpcgen -h -o
        ${CMAKE_CURRENT_BINARY_DIR}/blobs.h
        ${CMAKE_CURRENT_BINARY_DIR}/blobs.x
    COMMAND rpcgen -c -o
        ${CMAKE_CURRENT_BINARY_DIR}/blobs.c
        ${CMAKE_CURRENT_BINARY_DIR}/blobs.x
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/blobs.x
    )


add_definitions(
    -DOpenSpeedShop_LIBRARY_FILE_DIR="${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}"
)
  

# Do not build if only creating runtimes (RUNTIME_ONLY == true)
if (RUNTIME_ONLY MATCHES "false")

    add_library(usertime MODULE
	${PLUGIN_SOURCES}
    )

    target_include_directories(usertime PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/libopenss-runtime
	${PROJECT_SOURCE_DIR}/libopenss-framework
	${CMAKE_CURRENT_BINARY_DIR}/../../../libopenss-runtime
    )

    target_link_libraries(usertime
	pthread
	openss-framework
	${CMAKE_DL_LIBS}
    )

    set_target_properties(usertime PROPERTIES PREFIX "")

    install(TARGETS usertime
	LIBRARY DESTINATION lib${LIB_SUFFIX}/openspeedshop
    )

endif()

if (INSTRUMENTOR MATCHES "offline")

    add_definitions(
	-DOpenSpeedShop_LIBRARY_FILE_DIR="${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}"
	-DOPENSS_OFFLINE=1
    )

    if (RUNTIME_PLATFORM MATCHES "arm")
        add_definitions(
	    -fasynchronous-unwind-tables
	    -funwind-tables
        )
    endif()

    add_library(usertime-rt-offline SHARED
	${RT_SOURCES}
	offline.c
    )

    target_include_directories(usertime-rt-offline PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/libopenss-runtime
	${PROJECT_BUILD_DIR}/libopenss-runtime
	${PROJECT_SOURCE_DIR}/libopenss-runtime/offline
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}/../../../libopenss-runtime
    )

    target_link_libraries(usertime-rt-offline
	${CMAKE_DL_LIBS}
	rt
	pthread
	-Wl,-whole-archive
	openss-runtime-offline
	openss-runtime-unwind
	-Wl,-no-whole-archive
	${LibUnwind_LIBRARY_SHARED}
    )

    set_target_properties(usertime-rt-offline PROPERTIES COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};USE_FASTTRACE=1") 

    install(TARGETS usertime-rt-offline
	LIBRARY DESTINATION lib${LIB_SUFFIX}/openspeedshop
    )

    set_target_properties(usertime-rt-offline PROPERTIES PREFIX "")

    # Create and install static lib version
    add_library(usertime-offline STATIC
	${RT_SOURCES}
	offline.c
    )

    target_include_directories(usertime-offline PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/libopenss-runtime
	${PROJECT_BUILD_DIR}/libopenss-runtime
	${PROJECT_SOURCE_DIR}/libopenss-runtime/offline
	${LibMonitor_INCLUDE_DIRS}
	${LibUnwind_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}/../../../libopenss-runtime
    )

    target_link_libraries(usertime-offline
	${CMAKE_DL_LIBS}
	rt
	pthread
        -Wl,-whole-archive
	openss-runtime-offline
	openss-runtime-unwind
        -Wl,-no-whole-archive
    )

    set_target_properties(usertime-offline PROPERTIES COMPILE_DEFINITIONS "${TLS_DEFINES};${LibUnwind_DEFINES};USE_FASTTRACE=1") 

    install(TARGETS usertime-offline
	ARCHIVE DESTINATION lib${LIB_SUFFIX}/openspeedshop
    )

endif() # endif for INSTRUMENTOR MATCHES offline

file(COPY blobs.x DESTINATION .)
