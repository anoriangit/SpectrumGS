cmake_minimum_required(VERSION 3.14)

# Specify compiler (this needs to be at the top of this file)
set(CMAKE_C_COMPILER "/usr/bin/clang")

# Project name and language
project(spectrum-gs LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)


# Find SDL2 package
find_package(SDL2 REQUIRED)
if(SDL2_FOUND)
    message(STATUS "SDL2 Found: Yes")
    message(STATUS "SDL2 Include Directories: ${SDL2_INCLUDE_DIRS}")
    message(STATUS "SDL2 Libraries: ${SDL2_LIBRARIES}")
else()
    message(FATAL_ERROR "SDL2 not found! Please install SDL2 (e.g., 'sudo apt-get install libsdl2-dev' on Ubuntu).")
endif()

# get the root development directory (where Z80 and Zeta live)
get_filename_component(DEV_DIR ${CMAKE_CURRENT_LIST_DIR}/.. ABSOLUTE)

add_compile_options(
    -DZeta_WITH_CMAKE_SUPPORT=YES
)

# DEBUG build
# Set the build type to Debug for debugging symbols and no optimization
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-g)

# Add executable
add_executable(spectrum-gs 
    main.c text_box_l.c z80cpu.c z80mmu.c
    spectrum.c spectrum_ula.c cputraps.c
    spectrum_keyboard.c spectrum_text_overlay.c
    tapfile.c sdlut.c sdlevent.c
)


# Add all required libraries to the build
target_link_libraries(spectrum-gs
        Z80
        SDL2
)

# Add our include directories to the build
target_include_directories(spectrum-gs PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${DEV_DIR}/Z80/API
  ${DEV_DIR}/Zeta/API/Z
  ${SDL2_INCLUDE_DIRS}
)


# REDCODE Z80
# https://zxe.io/software/Z80/documentation/latest/integration.html

add_subdirectory("${DEV_DIR}/Zeta" "${DEV_DIR}/Zeta")
set(Zeta_INCLUDE_DIR                "Zeta")
set(Zeta_VERSION                    "v0.1")
set(Zeta_ROOT "Zeta")

set(Z80_SHARED_LIBS                 NO  CACHE BOOL "")
set(Z80_WITH_Q                      YES CACHE BOOL "")
set(Z80_WITH_ZILOG_NMOS_LD_A_IR_BUG YES CACHE BOOL "")

add_subdirectory("${DEV_DIR}/Z80" "${DEV_DIR}/Z80")
include_directories("${DEV_DIR}/Z80/API")



