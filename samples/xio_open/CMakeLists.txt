#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

compileAsC11()

add_executable(xio_open
	main.c)

include_directories(${SHARED_UTIL_INC_FOLDER})

set(xio_sample_libs)
list(APPEND xio_sample_libs
	aziotsharedutil)

if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)

	list(APPEND xio_sample_libs
		ws2_32
		secur32)

	if(${use_openssl})
		list(APPEND xio_sample_libs
			$ENV{OpenSSLDir}/lib/ssleay32.lib
			$ENV{OpenSSLDir}/lib/libeay32.lib)
	endif()
endif()

target_link_libraries(xio_open ${xio_sample_libs})

if(WIN32 AND ${use_openssl})
	file(COPY $ENV{OpenSSLDir}/bin/libeay32.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
	file(COPY $ENV{OpenSSLDir}/bin/ssleay32.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
endif()

set_target_properties(xio_open
			   PROPERTIES
			   FOLDER "azure_c_shared_utility_samples")
