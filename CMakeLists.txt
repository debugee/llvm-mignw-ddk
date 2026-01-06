cmake_minimum_required(VERSION 3.30)

project(testddk LANGUAGES C CXX)

if ((NOT CMAKE_C_COMPILER_ID STREQUAL "Clang") OR (NOT MINGW))
    message(FATAL_ERROR "Only support llvm-mingw")
endif()

# delete windows gui libraries
set(CMAKE_C_STANDARD_LIBRARIES "")
set(CMAKE_CXX_STANDARD_LIBRARIES "")

# delete -shared library flags
set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "")
set(CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS "")
set(CMAKE_SHARED_MODULE_CREATE_C_FLAGS "")

add_library(${PROJECT_NAME} SHARED test.c test.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
    C_STANDARD 11
    PREFIX ""
    SUFFIX ".sys"
)

target_compile_options(${PROJECT_NAME} PRIVATE -fvisibility=hidden)

# strip un referenced code and data
target_compile_options(${PROJECT_NAME} PRIVATE -ffunction-sections -fdata-sections)
target_link_options(${PROJECT_NAME} PRIVATE -Wl,--gc-sections)

# when include compiler builtin headers error
 target_compile_options(${PROJECT_NAME} PRIVATE
#     -fno-builtin
#     -mno-mmx
#     -mno-sse
#     -mno-sse2
    -ffreestanding
 )

# mingw headers not compatible with msvc
# let clang compatible with msvc
# target_compile_options(${PROJECT_NAME} PRIVATE -fms-compatibility -fms-extensions)
# target_compile_definitions(${PROJECT_NAME} PRIVATE -D_MSC_VER=1920 -D_MSC_EXTENSIONS)

#add c seh exceptions support, and __try/__except support
#https://github.com/llvm/llvm-project/issues/62606
#https://github.com/llvm/llvm-project/commit/797ad701522988e212495285dade8efac41a24d4
target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:-fseh-exceptions -fexceptions -fasync-exceptions>)
target_compile_options(${PROJECT_NAME} PRIVATE -fms-extensions) # enable __try/__except support

#wdm.h define and intrin-impl.h define conflict
target_compile_definitions(${PROJECT_NAME} PRIVATE
    -D__INTRINSIC_DEFINED_InterlockedBitTestAndSet
    -D__INTRINSIC_DEFINED_InterlockedBitTestAndReset
)

# disable tsaware is very important
target_link_options(${PROJECT_NAME} PRIVATE
    -nostdlib
    -Wl,--entry=DriverEntry
    -Wl,--subsystem=native
    -Wl,--file-alignment,0x1000
    -Wl,--disable-tsaware
    -Wl,/integritycheck
)

find_file(NTDDK_HEADER_FILE ddk/ntddk.h PATHS ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
cmake_path(GET NTDDK_HEADER_FILE PARENT_PATH DDK_DIR)
target_include_directories(${PROJECT_NAME} PRIVATE ${DDK_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE ntoskrnl)

# sign the driver
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -f $<TARGET_FILE_BASE_NAME:${PROJECT_NAME}>_signed.sys
    COMMAND osslsigncode sign
    -pass 1234
    -pkcs12 ${CMAKE_CURRENT_SOURCE_DIR}/user.pfx
    -n "DriverSign"
    -in $<TARGET_FILE:${PROJECT_NAME}>
    -out $<TARGET_FILE_BASE_NAME:${PROJECT_NAME}>_signed.sys
    VERBATIM
)

add_executable(ioctl ioctl.c)