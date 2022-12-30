set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")

project(cpp_template)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

if(MSVC)
  # Without a byte order mark (BOM), Visual Studio assumes that the source file
  # is encoded using the current user code page, so we specify UTF-8.
  add_compile_options(/utf-8)
endif(MSVC)

add_subdirectory(extern)
add_subdirectory(src)
enable_testing()
add_subdirectory(tests)
