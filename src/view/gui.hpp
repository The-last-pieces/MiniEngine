//
// Created by IMEI on 2022/7/13.
//

#ifndef MINI_ENGINE_GUI_HPP
#define MINI_ENGINE_GUI_HPP

#include "GLFW/glfw3.h"
#include <string>
#include "color.hpp"

namespace mne {
class MainWindow {
protected:
    GLFWwindow* window = nullptr;

private:
    // 标题
    std::string_view title;

    // 宽高
    int width, height;
    // 背景颜色
    Color bg_color = {0, 0, 0};

public:
    MainWindow(std::string_view title, int width, int height):
        title(title), width(width), height(height) {
        glfwInit(); // 初始化
    }

    ~MainWindow() {
        glfwTerminate(); // 终止并释放GLFW资源
    }

    // 阻塞直到窗口关闭
    void show() {
        // glfw 创建窗口
        window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
        if (window == nullptr) return;
        // glfw 创建运行上下文
        glfwMakeContextCurrent(window);
        // glfw 事件循环
        while (!glfwWindowShouldClose(window)) {
            clearWith(bg_color); // 清除颜色缓存
            update();            // 更新ui

            glfwSwapBuffers(window); // 实现双缓冲
            glfwPollEvents();        // 处理IO事件(键盘,鼠标...)
        }
        glfwDestroyWindow(window);
        window = nullptr;
    }

protected:
    // mode控制绘图类型如:
    // GL_TRIANGLES,GL_LINES,GL_POINTS
    // 分别表示:
    // 三角形,线段,像素
#define DRAW_WITH_MODE(mode, code) \
    do {                           \
        glBegin(mode);             \
        {code};                    \
        glEnd();                   \
    } while (false)

    virtual void update() {
        // 绘制OpenGL经典图案
        DRAW_WITH_MODE(GL_TRIANGLES, {
            glColor3f(1.0, 0.0, 0.0); // Red
            glVertex3f(0.0, 1.0, 0.0);

            glColor3f(0.0, 1.0, 0.0); // Green
            glVertex3f(-1.0, -1.0, 0.0);

            glColor3f(0.0, 0.0, 1.0); // Blue
            glVertex3f(1.0, -1.0, 0.0);
        });
    }

protected:
    // 封装常用的OpenGL操作
    static void clearWith(const Color& color) {
        // 设置背景色
        glClearColor(color.r, color.g, color.b, 0.0);
        // 以设置的背景色填充颜色缓冲区
        glClear(GL_COLOR_BUFFER_BIT);
    }

#undef DRAW_WITH_MODE
};
} // namespace mne

#endif