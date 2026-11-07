cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME Mini_Blance_Car_WHEELTEC_B570)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
    Core/Src/Hardware/Blue.c
    Core/Src/Hardware/Blue.h
    Core/Src/Hardware/OLED.c
    Core/Src/Hardware/OLED.h
    Core/Src/Hardware/Motor.c
    Core/Src/Hardware/Motor.h
    Core/Src/Algorithm/pid.c
    Core/Src/Algorithm/pid.h
    Core/Src/Hardware/Encoder.c
    Core/Src/Hardware/Encoder.h
    Core/Src/Hardware/MPU6050/MPU6050.c
    Core/Src/Hardware/MPU6050/inv_mpu.c
    Core/Src/Hardware/MPU6050/inv_mpu_dmp_motion_driver.c
    Core/Src/Hardware/MPU6050/ioi2c.c
    Core/Src/Hardware/MPU6050/delay.c)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Src/Hardware
    Core/Src/Hardware/MPU6050
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)