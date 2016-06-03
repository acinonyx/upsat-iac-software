cmake_minimum_required(VERSION 2.8)

include(GNUInstallDirs)
#include(FindImageMagick
#  RESULT_VARIABLE FindImageMagick)

add_definitions(
  -ggdb
  -Wall
  -Werror
  -Wconversion
  -Wmissing-declarations
  -Wmissing-prototypes
  -Wstrict-prototypes
  -Wcast-qual
  -Wcast-align
  -Wshadow
  -pedantic
  --std=gnu99
  )
add_definitions(
  -DMAGICKCORE_QUANTUM_DEPTH=16
  -DMAGICKCORE_HDRI_ENABLE=0
  )

#find_package(ImageMagick
#  REQUIRED)
#  COMPONENTS MagickWand)

find_path(ImageMagick_MagickWand_INCLUDE_DIRS
  NAMES wand/magick_wand.h
  PATHS /usr/include/ImageMagick
  )
find_library(ImageMagick_MagickWand_LIBRARIES
  REQUIRED
  NAMES MagickWand MagickWand-6.Q16
  )

include_directories(SYSTEM ${ImageMagick_MagickWand_INCLUDE_DIRS})
link_directories(${ImageMagick_MagickWand_LIBRARIES})

find_path(xiApi_INCLUDE_DIRS
  NAMES xiApi.h
  PATHS /opt/XIMEA/include
  )
find_library(xiApi_LIBRARIES
  REQUIRED
  NAMES m3api
  )
include_directories(SYSTEM ${xiApi_INCLUDE_DIRS})
link_directories(${xiApi_LIBRARIES})

set(SOURCES iac.c camera.c image.c spi.c)
set(LIBS m3api MagickWand)

if(DEBUG)
  add_definitions(-DDEBUG -g3)
endif()

add_executable(iac ${SOURCES})
target_link_libraries(iac ${LIBS})
install(TARGETS iac RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
