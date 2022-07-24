cmake_minimum_required(VERSION 3.0)

project(mini_engine)

# 输出路径
## exe
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/dist)
## lib
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/dist)

# 编译选项
## 使用C++20标准
set(CMAKE_CXX_STANDARD 20)
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    add_compile_options(-fconcepts)
endif ()
## 添加OpenMP选项
FIND_PACKAGE(OpenMP REQUIRED)
if (OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

# 拷贝资源文件
file(COPY ./res DESTINATION ${EXECUTABLE_OUTPUT_PATH})

# 源文件集合
set(SOURCE_FILES
        src/engine/camera.hpp
        src/engine/model.hpp
        src/engine/render.hpp
        src/engine/shader.hpp
        src/math/mat.hpp
        src/math/utils.hpp
        src/math/vec.hpp
        src/view/color.hpp
        src/view/gui.hpp
        src/view/image.hpp
        src/main.cpp)

# 库目录, -L
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 头文件目录, -I
include_directories(${PROJECT_SOURCE_DIR}/include)

# 源文件
add_executable(main ${SOURCE_FILES})

# 依赖库
target_link_libraries(main opengl32 glfw3)