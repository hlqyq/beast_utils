################################################################################
## CMake options ###############################################################
################################################################################

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)  # setting this is required
set_property(GLOBAL PROPERTY USE_FOLDERS ON)  # Better IDE support: turn on using solution folders
set(CMAKE_VERBOSE_MAKEFILE ON)  # Debug friendly

################################################################################
## Project options #############################################################
################################################################################

project(beast_utils VERSION 1.0.0 LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

################################################################################
## variables definitions #######################################################
################################################################################

set(BASE_DIRECTORY ${PROJECT_SOURCE_DIR}/base)
set(SOURCE_DIRECTORY ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/include)
set(EXPORT_DIRECTORY ${PROJECT_SOURCE_DIR}/export)
set(NET_DIRECTORY ${PROJECT_SOURCE_DIR}/net)
set(OS_DIRECTORY ${PROJECT_SOURCE_DIR}/os_glue)
set(BIN_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

###############################################################################
## file globbing ##############################################################
###############################################################################

# Set up common source code
set(SOURCES
    ${BASE_DIRECTORY}/console_close.cpp
    ${BASE_DIRECTORY}/utils.cpp
    ${EXPORT_DIRECTORY}/beast_utils_export.cpp
    ${INCLUDE_DIRECTORY}/beast_utils.h
    ${NET_DIRECTORY}/detect_session.cpp
    ${NET_DIRECTORY}/http_session_plain.cpp
    ${NET_DIRECTORY}/http_session_ssl.cpp
    ${NET_DIRECTORY}/listener.cpp
    ${NET_DIRECTORY}/net_utils.cpp
    ${SOURCE_DIRECTORY}/app_resource.cpp
    ${SOURCE_DIRECTORY}/scaffold_handles.cpp
    ${SOURCE_DIRECTORY}/ssl_certificate.cpp
)

# Set up platform dependent source code
if(WIN32)
    list(APPEND SOURCES ${OS_DIRECTORY}/windows/os_glue_windows.cpp)
elseif(UNIX AND NOT APPLE AND NOT ANDROID)
    list(APPEND SOURCES ${OS_DIRECTORY}/linux/os_glue_linux.cpp)
endif()

# Set up platform dependent compile temporary files
if(WIN32)
    set(COMPILE_TEMP_FILES
        ${BIN_DIRECTORY}/${PROJECT_NAME}.ilk
        ${BIN_DIRECTORY}/${PROJECT_NAME}.pdb
    )
endif()

###############################################################################
## dependencies ###############################################################
###############################################################################

# Set up platform dependent variables of boost library
if(WIN32)
    # set(Boost_DEBUG ON)
    set(Boost_NO_BOOST_CMAKE ON)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME OFF)
    add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
endif()

# find libraries
find_package(Boost REQUIRED COMPONENTS date_time system regex)
find_package(OpenSSL REQUIRED)

if(Boost_FOUND)
  message(STATUS "Boost(${Boost_VERSION_STRING}) has found:")
  message(STATUS "    Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
  message(STATUS "    Boost_INCLUDE_DIR: ${Boost_INCLUDE_DIR}")
  message(STATUS "    Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
  message(STATUS "    Boost_LIBRARIES: ${Boost_LIBRARIES}")
  message(STATUS "    Boost_VERSION_COUNT: ${Boost_VERSION_COUNT}")
  message(STATUS "    Boost_LIB_DIAGNOSTIC_DEFINITIONS: ${Boost_LIB_DIAGNOSTIC_DEFINITIONS}")
else()
  if(WIN32)
    message(WARNING "Boost hasn't found!, Please set [BOOST_ROOT] environment variable to the directory of an boost installation.")
  else()
    message(WARNING "Boost hasn't found in ${CMAKE_SYSTEM}!")
  endif()
endif()

if(OPENSSL_FOUND)
  message(STATUS "OpenSSL(${OPENSSL_VERSION}) has found:")
  message(STATUS "    OPENSSL_INCLUDE_DIR: ${OPENSSL_INCLUDE_DIR}")
  message(STATUS "    OPENSSL_CRYPTO_LIBRARY: ${OPENSSL_CRYPTO_LIBRARY}")
  message(STATUS "    OPENSSL_CRYPTO_LIBRARIES: ${OPENSSL_CRYPTO_LIBRARIES}")
  message(STATUS "    OPENSSL_SSL_LIBRARY: ${OPENSSL_SSL_LIBRARY}")
  message(STATUS "    OPENSSL_SSL_LIBRARIES: ${OPENSSL_SSL_LIBRARIES}")
  message(STATUS "    OPENSSL_LIBRARIES: ${OPENSSL_LIBRARIES}")
  message(STATUS "    OPENSSL_APPLINK_SOURCE: ${OPENSSL_APPLINK_SOURCE}")
else()
  if(WIN32)
    message(WARNING "OpenSSL hasn't found!, Please set [OPENSSL_ROOT_DIR] environment variable to the directory of an boost installation.")
  else()
    message(WARNING "OpenSSL hasn't found in ${CMAKE_SYSTEM}!")
  endif()
endif()

###############################################################################
## Compiling pre-options ######################################################
###############################################################################

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

###############################################################################
## target definitions #########################################################
###############################################################################

add_library(${PROJECT_NAME} SHARED ${SOURCES})  # Set up compile type: dynamic shared library

target_include_directories(${PROJECT_NAME}
    PUBLIC
        include
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${OpenSSL_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

###############################################################################
## Compiling options ##########################################################
###############################################################################

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

# Set up platform dependent compile options
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BEASTUTILS_EXPORTS)
elseif(UNIX AND NOT APPLE AND NOT ANDROID)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BEASTUTILS_EXPORTS)
elseif(APPLE)
else()
endif()

# Set up platform dependent variables
if(WIN32)
    message(STATUS "compile in target platform: Windows")
    add_definitions(-D_WIN32_WINNT=0x0601)  # Windows7
    add_definitions(/bigobj)
elseif(UNIX AND NOT APPLE AND NOT ANDROID)
    message(STATUS "compile in target platform: linux")
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_C_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
elseif(APPLE)
    message(STATUS "compile in target platform: MacOS")
else()
    message(STATUS "compile in target platform: ${CMAKE_SYSTEM}")
endif()

###############################################################################
## link options ###############################################################
###############################################################################

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/bin/"
)

target_link_libraries(${PROJECT_NAME}
    ${Boost_LIBRARIES}
    ${OpenSSL_LIBRARIES}
    OpenSSL::SSL
)

LINK_LIBRARIES(${PROJECT_NAME} ${Boost_LIBRARY_DIRS})

###############################################################################
## Compile temporary files remove #############################################
###############################################################################

if(COMPILE_TEMP_FILES)
    file(REMOVE ${COMPILE_TEMP_FILES})
endif()

###############################################################################
## packaging ##################################################################
###############################################################################

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
