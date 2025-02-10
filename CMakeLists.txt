cmake_minimum_required(VERSION 3.10)
project(netc C)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -O3 -Ofast")

# Include directories
include_directories(include)

# Find source files
set(SRCS src/netc.c src/raw/raw.c)
set(LIBS)

# Option to link with OpenSSL
option(USE_OPENSSL "Link with OpenSSL" ON)
if (USE_OPENSSL)
  list(APPEND SRCS src/ssl/openssl.c)
  list(APPEND LIBS ${CMAKE_SOURCE_DIR}/openssl/libssl.a ${CMAKE_SOURCE_DIR}/openssl/libcrypto.a)
  include_directories(${CMAKE_SOURCE_DIR}/openssl/include)
endif()

# Create shared library
add_library(netc SHARED ${SRCS})
if (USE_OPENSSL)
  target_link_libraries(netc ${LIBS})
endif()

# Create static library
add_library(netc_static STATIC ${SRCS})
if (USE_OPENSSL)
  target_link_libraries(netc_static ${LIBS})
endif()

include_directories(${CMAKE_SOURCE_DIR})

# Install headers and library
install(DIRECTORY include/ DESTINATION /usr/local/include)
install(TARGETS netc netc_static
  LIBRARY DESTINATION /usr/local/lib
  ARCHIVE DESTINATION /usr/local/lib
)

