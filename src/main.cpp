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

typedef enum
{
    NONE,
    BUFFER,
    APPLY,
    WRITE,
    TOGREYSCALE,
    TONEGATIVE,
    SCALECONTRAST,
    COLORTINT,
    THRESHOLD,
    ADJUSTHSV,
    ROTATEHUE,
    SCALESATURATION,
    SCALEVALUE,
    GAUSSIANBLUR,
    BLOOM
} IMGEditType;


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

void loadTempTexture(ImageF& image, GLuint& texture)
{
    if (texture)
        glDeleteTextures(1, &texture);
    glGenTextures(1, &texture);
    glBindTexture(1, texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_FLOAT, image.temp);
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
    glfwSwapInterval(1); // Enable vsync

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

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImageF image;
    GLuint inputTexture = 0;
    GLuint outputTexture = 0;
    bool reloadTexture = false;
    bool reloadTextureTemp = false;
    float threshold = 0.5f;
    float greyR = 0.33f;
    float greyG = 0.33f;
    float greyB = 0.33f;
    float tintR = 1.0f;
    float tintG = 0.0f;
    float tintB = 0.0f;
    float tintA = 0.10f;
    float contrastLower = 0.0f;
    float contrastUpper = 1.0f;
    char readFilename[1024] = "";
    char writeFilename[1024] = "";
    float h = 0.0f;
    float s = 1.0f;
    float v = 1.0f;
    int kernel = 61;
    float bloomThreshold = 0.5f;
    int bloomKernel = 61;

    bool textureLoaded = false;
    bool updateComplete = false;
    std::atomic<IMGEditType> editType = NONE;
    std::thread imageProcessingThread([&]()
    {
        bool runThread = true;
        while (runThread)
        {
            switch (editType)
            {
                case NONE:
                    break;
                case BUFFER:
                    image.buffer(readFilename);
                    reloadTexture = true;
                    break;
                case APPLY:
                    image.apply();
                    reloadTexture = true;
                    reloadTextureTemp = true;
                    // Prevent double swapping
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    break;
                case WRITE:
                    image.write(writeFilename);
                    reloadTexture = true;
                    reloadTextureTemp = true;
                    break;
                case TOGREYSCALE:
                    image.toGreyscale(greyR, greyG, greyB);
                    reloadTextureTemp = true;
                    break;
                case TONEGATIVE:
                    image.toNegative();
                    reloadTextureTemp = true;
                    break;
                case SCALECONTRAST:
                    image.scaleContrast(contrastLower, contrastUpper);
                    reloadTextureTemp = true;
                    break;
                case COLORTINT:
                    image.colorTint(tintR, tintB, tintG, tintA);
                    reloadTextureTemp = true;
                    break;
                case THRESHOLD:
                    image.threshold(threshold);
                    reloadTextureTemp = true;
                    break;
                case ADJUSTHSV:
                    image.adjustHSV(h, s, v);
                    reloadTextureTemp = true;
                    break;
                case GAUSSIANBLUR:
                    image.gaussianBlur(kernel);
                    reloadTextureTemp = true;
                    break;
                case BLOOM:
                    image.bloom(bloomThreshold, bloomKernel);
                    reloadTextureTemp = true;
                    break;
                default:
                    break;
            }
        }
    });
    imageProcessingThread.detach();


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

        if (reloadTexture)
        {
            loadTexture(image, inputTexture);
            reloadTexture = false;
            textureLoaded = true;
            editType = NONE;
        }

        if (reloadTextureTemp)
        {
            loadTempTexture(image, outputTexture);
            reloadTextureTemp = false;
            editType = NONE;
        }

        // Main window
        {
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
;
            ImGui::Begin("Compositor!");
            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::Text("Application average %.ef ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::InputText("File Name", readFilename, sizeof(readFilename));
            ImGui::SameLine();
            if (ImGui::Button("Open File"))
            {
                editType = BUFFER;
            }

            if (textureLoaded)
            {
                static float imgWidth = 1000.0f;
                static float imgHeight = imgWidth / image.aspectRatio;
                ImGui::Image((void *)(intptr_t)inputTexture, ImVec2(imgWidth, imgHeight));
                ImGui::SameLine();
                ImGui::Image((void *)(intptr_t)outputTexture, ImVec2(imgWidth, imgHeight));
            }

            static IMGEditType editor = NONE;
            ImGui::Text("Choose an edit to make:");
            ImGui::RadioButton("Make Greyscale", (int*)&editor, (int)TOGREYSCALE);
            ImGui::SameLine();
            ImGui::RadioButton("Make Negative", (int*)&editor, (int)TONEGATIVE);
            ImGui::SameLine();
            ImGui::RadioButton("Adjust Contrast", (int*)&editor, (int)SCALECONTRAST);
            ImGui::SameLine();
            ImGui::RadioButton("Adjust Tint", (int*)&editor, (int)COLORTINT);
            ImGui::SameLine();
            ImGui::RadioButton("Threshold Image", (int*)&editor, (int)THRESHOLD);
            ImGui::SameLine();
            ImGui::RadioButton("Adjust HSV", (int*)&editor, (int)ADJUSTHSV);
            ImGui::SameLine();
            ImGui::RadioButton("Gaussian Blur Image", (int*)&editor, (int)GAUSSIANBLUR);
            ImGui::SameLine();
            ImGui::RadioButton("Add Bloom", (int*)&editor, (int)BLOOM);

            switch (editor)
            {
                case TOGREYSCALE:
                    ImGui::Text("Choose different weights for converting to greyscale");

                    if(ImGui::SliderFloat("r", &greyR, 0.0f, 1.0f)
                       | ImGui::SliderFloat("g", &greyG, 0.0f, 1.0f)
                       | ImGui::SliderFloat("b", &greyB, 0.0f, 1.0f))
                    {
                        editType = TOGREYSCALE;
                    }
                    break;
                case TONEGATIVE:
                    if (ImGui::Button("Negate Image"))
                    {
                        editType = TONEGATIVE;
                    }
                    break;
                case SCALECONTRAST:
                    if (ImGui::SliderFloat("Lower Bound", &contrastLower, -1.0f, 5.0f)
                        | ImGui::SliderFloat("Upper Bound", &contrastUpper, 0.0f, 4.0f))
                    {
                        editType = SCALECONTRAST;
                    }
                    break;
                case COLORTINT:
                    if (ImGui::SliderFloat("R", &tintR, 0.0f, 1.0f)
                        | ImGui::SliderFloat("G", &tintG, 0.0f, 1.0f)
                        | ImGui::SliderFloat("B", &tintB, 0.0f, 1.0f)
                        | ImGui::SliderFloat("A", &tintA, 0.0f, 1.0f))
                    {
                        editType = COLORTINT;
                    }
                    break;
                case THRESHOLD:
                    if (ImGui::SliderFloat("Threshold", &threshold, 0.0f, 1.0f))
                    {
                        editType = THRESHOLD;
                    }
                    break;
                case ADJUSTHSV:
                    if (ImGui::SliderFloat("Hue", &h, 0.0f, 360.0f)
                        | ImGui::SliderFloat("Saturation", &s, 0.0f, 1.0f)
                        | ImGui::SliderFloat("Value", &v, 0.0f, 1.0f))
                    {
                        editType = ADJUSTHSV;
                    }
                    break;
                case GAUSSIANBLUR:
                    if (ImGui::SliderInt("Kernel Size", &kernel, 0, 500))
                    {
                        editType = GAUSSIANBLUR;
                    }
                    break;
                case BLOOM:
                    if (ImGui::SliderInt("Bloom Kernel", &bloomKernel, 0, 500)
                        | ImGui::SliderFloat("Bloom Threshold", &bloomThreshold, 0.0f, 1.0f))
                    {
                        editType = BLOOM;
                    }
                    break;
                default:
                    break;
            }

                    
            
            if (ImGui::Button("Apply Changes"))
            {
                editType = APPLY;
            }

            ImGui::InputText("File Output Name", writeFilename, sizeof(writeFilename));
            ImGui::SameLine();
            if (ImGui::Button("Write File"))
            {
                editType = WRITE;
            }
            ImGui::End();
        }

        // Start rendering
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
    if (inputTexture)
        glDeleteTextures(1, &inputTexture);
    if (outputTexture)
        glDeleteTextures(1, &outputTexture);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}
