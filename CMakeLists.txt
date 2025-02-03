cmake_minimum_required(VERSION 3.27.0)

project(MCC)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
pkg_check_modules(SDL2NET REQUIRED SDL2_net)
pkg_check_modules(SDL2MIXER REQUIRED SDL2_mixer)

if(SDL2_FOUND)
  message(STATUS "SDL2 FOUND")
  set(SDL2_TTF "SDL2_ttf")
endif()

if(SDL2NET_FOUND)
  message(STATUS "SDL2_net FOUND")
endif()
if(SDL2MIXER_FOUND)
  message(STATUS "SDL2_mixer Found" )
endif()
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lSDL2_mixer -Wl,--allow-multiple-definition -g -L../libs/Box2d/lib -L../libs/qrcodegen/lib")
set(BOX2D_LIB "box2d")
set(QRCODEGEN_LIB "qrcodegencpp")

file(GLOB SRC_FILES
  ${PROJECT_SOURCE_DIR}/src/**/*.cpp
)

add_executable(
  ${PROJECT_NAME}
  ${PROJECT_SOURCE_DIR}/apps/main.cpp
  ${SRC_FILES}
)

include_directories(
  ${SDL2_INCLUDE_DIRS} 
  ${SDL2NET_INCLUDE_DIRS}
  ${SDL2IMAGE_INCLUDE_DIRS}
  ${SDL2MIXER_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(
  ${PROJECT_NAME} 
  ${SDL2_LIBRARIES} 
  ${SDL2NET_LIBRARIES}
  ${SDL2IMAGE_LIBRARIES} 
  ${SDL2_TTF} 
  ${SDL2MIXER_LIBRARIES}
  ${BOX2D_LIB} 
  ${QRCODEGEN_LIB} 
)