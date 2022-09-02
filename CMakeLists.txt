cmake_minimum_required(VERSION 3.0)

project(mini_engine)

# 输出路径
## exe
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/dist)
## lib
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/dist)

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
## 关闭_s警告
if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()

# 拷贝资源文件
file(COPY external/art DESTINATION ${EXECUTABLE_OUTPUT_PATH})

# 源文件集合
set(SOURCE_FILES
        src/engine/data/camera.hpp
        src/engine/data/color.hpp
        src/engine/data/ray.hpp
        src/engine/data/scene.hpp

        src/engine/store/image.hpp
        src/engine/store/model.hpp

        src/engine/accelerator/AABB.hpp

        src/engine/interface/render.hpp
        src/engine/interface/shader.hpp
        src/engine/interface/material.hpp
        src/engine/interface/object.hpp
        src/engine/interface/texture.hpp

        src/engine/implement/material/default.hpp
        src/engine/implement/material/diffuse.hpp
        src/engine/implement/material/mirror.hpp
        src/engine/implement/material/refraction.hpp
        src/engine/implement/material/micro.hpp
        src/engine/implement/material/disney.hpp

        src/engine/implement/objects/sphere.hpp
        src/engine/implement/objects/rectangle.hpp

        src/engine/math/mat.hpp
        src/engine/math/utils.hpp
        src/engine/math/vec.hpp

        src/engine/implement/render/rs_render.hpp
        src/engine/implement/render/rt_render.hpp

        src/engine/implement/texture/solid.hpp
        src/engine/implement/texture/mapping.hpp

        src/engine/tools/average.hpp
        src/engine/tools/process.hpp
        src/engine/tools/json.hpp

        src/view/gui.hpp

        src/main.cpp)

# 库目录, -L
link_directories(external/lib)

# 头文件目录, -I
include_directories(external/include src/engine) 

# 源文件
add_executable(main ${SOURCE_FILES})

# 依赖库
target_link_libraries(main opengl32 glfw3)