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
        src/engine/data/camera.hpp
        src/engine/data/color.hpp
        src/engine/data/ray.hpp
        src/engine/data/scene.hpp

        src/engine/store/image.hpp
        src/engine/store/model.hpp
        src/engine/store/texture.hpp

        src/engine/accelerator/AABB.hpp

        src/engine/interface/render.hpp
        src/engine/interface/shader.hpp
        src/engine/interface/material.hpp
        src/engine/interface/object.hpp

        src/engine/material/diffuse.hpp
        src/engine/material/default.hpp

        src/engine/objects/sphere.hpp
        src/engine/objects/rectangle.hpp

        src/engine/math/mat.hpp
        src/engine/math/utils.hpp
        src/engine/math/vec.hpp

        src/engine/render/rs_render.hpp
        src/engine/render/rt_render.hpp

        src/view/gui.hpp

        src/main.cpp)

# 库目录, -L
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 头文件目录, -I
include_directories(${PROJECT_SOURCE_DIR}/include)

# 源文件
add_executable(main ${SOURCE_FILES})

# 依赖库
target_link_libraries(main opengl32 glfw3)