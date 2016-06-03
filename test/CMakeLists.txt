cmake_minimum_required(VERSION 2.8)

include(GNUInstallDirs)

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

find_path(iac_INCLUDE_DIRS
  NAMES iac.h spi.h
  PATHS ${PROJECT_SOURCE_DIR}/src
  )

include_directories(SYSTEM ${iac_INCLUDE_DIRS})

set(SOURCES iac-spi-test.c ${PROJECT_SOURCE_DIR}/src/spi.c)

if(DEBUG)
  add_definitions(-DDEBUG -g3)
endif()

add_executable(iac-spi-test ${SOURCES})
target_link_libraries(iac-spi-test ${LIBS})
install(TARGETS iac-spi-test RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
