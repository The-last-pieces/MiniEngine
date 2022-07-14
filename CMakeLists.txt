cmake_minimum_required(VERSION 3.0)

project(mini_engine)

# 编译选项
set(CMAKE_CXX_STANDARD 17)

# 输入文件
set(SOURCE_FILES
        src/engine/data.hpp
        src/engine/model.hpp
        src/engine/render.hpp
        src/engine/shader.hpp
        src/math/mat.hpp
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