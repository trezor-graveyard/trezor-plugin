#/**********************************************************\
# Auto-generated X11 project definition file for the
# Bitcoin Trezor Plugin project
#\**********************************************************/

# X11 template platform definition CMake file
# Included from ../CMakeLists.txt

include(X11/Findlibusb-1.0.cmake)
include(X11/Findiconv.cmake)

find_library(LIBUSB libusb-1.0)
find_library(RT_LIBRARY rt)
find_library(ICONV_LIBRARIES iconv)

include_directories(${LIBUSB_1_INCLUDE_DIRS})
include_directories(${RT_INCLUDE_DIRS})
include_directories(${ICONV_INCLUDE_DIRS})
include_directories(${GTK_INCLUDE_DIRS}

# remember that the current source dir is the project root; this file is in X11/
file (GLOB PLATFORM RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    X11/[^.]*.cpp
    X11/[^.]*.c
    X11/[^.]*.h
    X11/[^.]*.cmake
    )

SOURCE_GROUP(X11 FILES ${PLATFORM})

# use this to add preprocessor definitions
add_definitions(
)

set (SOURCES
    ${SOURCES}
    ${PLATFORM}
    )

add_x11_plugin(${PROJECT_NAME} SOURCES)

# add library dependencies here; leave ${PLUGIN_INTERNAL_DEPS} there unless you know what you're doing!
target_link_libraries(${PROJECT_NAME}
    ${PLUGIN_INTERNAL_DEPS}
    ${LIBUSB_1_LIBRARIES}
    ${RT_LIBRARY}
    ${ICONV_LIBRARIES}
    -lz
    )
