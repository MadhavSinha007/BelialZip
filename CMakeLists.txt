# 1. Setup the project and minimum CMake version required
cmake_minimum_required(VERSION 3.16)
project(belialzip VERSION 1.0.0 LANGUAGES CXX)

# 2. Tell the compiler to use C++17 (a modern version of C++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 3. Tell the compiler where to find our "menu" (header files)
include_directories(include)

# 4. Find all .cpp files in the src/ folder, but EXCLUDE main.cpp for now
file(GLOB_RECURSE BELIALZIP_SOURCES "src/*.cpp")
list(FILTER BELIALZIP_SOURCES EXCLUDE REGEX "main\\.cpp")

# 5. Bundle all those .cpp files into a "Library" called belialzip_lib
add_library(belialzip_lib STATIC ${BELIALZIP_SOURCES})

# 6. We will eventually need this executable, so let's prepare it
add_executable(belialzip src/main.cpp)
target_link_libraries(belialzip PRIVATE belialzip_lib)

# 7. Turn on testing and tell CMake to look in the tests/ folder
enable_testing()
add_subdirectory(tests)