cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeModules/")

include(GNUInstallDirs)
include(FindImageMagick
  RESULT_VARIABLE FindImageMagick)
include(FindXiApi
  RESULT_VARIABLE FindXiApi)

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

find_package(ImageMagick
  REQUIRED
  COMPONENTS MagickWand
  )
find_package(XiApi
  REQUIRED
  )

include_directories(
  SYSTEM ${ImageMagick_MagickWand_INCLUDE_DIR}
  SYSTEM ${XiApi_INCLUDE_DIR}
  )

set(SOURCES iac.c camera.c image.c spi.c obc.c utils.c)
set(LIBS ${ImageMagick_MagickWand_LIBRARY} ${XiApi_LIBRARY})

if(DEBUG)
  add_definitions(-DDEBUG -g3)
endif()

add_executable(iac ${SOURCES})
target_link_libraries(iac ${LIBS})
install(TARGETS iac RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
