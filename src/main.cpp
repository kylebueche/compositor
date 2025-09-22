/*******************************************************
 * Author: Kyle Bueche
 * File: main.cpp
 * 
 * Image manipulator
 ******************************************************/
#include <iostream>
#include "image.h"
#include <limits>
#include <string>
#include <thread>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define GL_SILENCE_DEPRACATION
#include <GLFW/glfw3.h>

static void glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void loadTexture(ImageF& image, GLuint& texture)
{
    if (texture)
        glDeleteTextures(1, &texture);
    glGenTextures(1, &texture);
    glBindTexture(1, texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_FLOAT, image.pixels);
}

int main()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;
    
    const char* glsl_version = "#version 450";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    GLFWwindow* window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "Compositor", nullptr, nullptr);

    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    GLuint texture = 0;

    char textBuffer[1024] = "";

    ImageF image;
    image.buffer("image.jpg");

    std::cout << "hi";

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        {
            static float r = 0.0f;
            static float g = 0.0f;
            static float b = 0.0f;
            static bool textureLoaded = false;
            ImGui::Begin("Hello, world!");
            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::Text("Choose different weights for converting to greyscale");
            ImGui::SliderFloat("r", &r, 0.0f, 1.0f);
            ImGui::SliderFloat("g", &g, 0.0f, 1.0f);
            ImGui::SliderFloat("b", &b, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            ImGui::InputText("File Name", textBuffer, sizeof(textBuffer));
            if (ImGui::Button("Open File"))
            {
                std::cout << textBuffer;
                image.buffer(textBuffer);
                std::cout << "hi";
                loadTexture(image, texture);
                textureLoaded = true;
            }
            
            if (textureLoaded)
            {
                ImGui::Image((void *)(intptr_t)texture, ImVec2(image.width, image.height));
            }
            
            if (ImGui::Button("Button"))
            {
                image.toGreyscale(r, g, b);
                loadTexture(image, texture);
            }
            ImGui::SameLine();
            ImGui::Text("Application average %.ef ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello another window");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}
