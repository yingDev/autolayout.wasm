cmake_minimum_required(VERSION 3.12)
project(autolayout.wasm)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(BOOST_SPIRIT_X3_NO_RTTI BOOST_SPIRIT_NO_REAL_NUMBERS BOOST_SPIRIT_NO_STANDARD_WIDE)
include_directories(kiwi/kiwi ${BOOST_ROOT}/include)

if (DEFINED EMSCRIPTEN)

    set(CMAKE_CXX_FLAGS "-flto=full -fno-rtti --llvm-lto 3  --bind --closure 1 --memory-init-file 0 -s WASM=1 --post-js '../post.js' -s STRICT=1 -s MODULARIZE=1 -s EXPORT_ES6=1 -s EXPORT_NAME=AutoLayout -s MODULARIZE_INSTANCE=1 -s ENVIRONMENT=web -s FILESYSTEM=0 -s NO_EXIT_RUNTIME=1  -s USE_PTHREADS=0 -s ELIMINATE_DUPLICATE_FUNCTIONS=1 -O3 -DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0")
    add_executable(autolayout em_autolayout.cpp em_evfl.cpp)

else (DEFINED EMSCRIPTEN)

    #set(CMAKE_CXX_FLAGS "-flto=full")
    add_executable(autolayout evfl/test.cpp)

endif (DEFINED EMSCRIPTEN)
