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
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define GL_SILENCE_DEPRACATION
#include "glad.h"
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
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    glfwSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImageF image;
    GLuint inputTexture = 0;
    bool reloadTexture = false;
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
                    // Prevent double swapping
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    break;
                case WRITE:
                    image.write(writeFilename);
                    reloadTexture = true;
                    break;
                case TOGREYSCALE:
                    image.toGreyscale(greyR, greyG, greyB);
                    reloadTexture = true;
                    break;
                case TONEGATIVE:
                    image.toNegative();
                    reloadTexture = true;
                    break;
                case SCALECONTRAST:
                    image.scaleContrast(contrastLower, contrastUpper);
                    reloadTexture = true;
                    break;
                case COLORTINT:
                    image.colorTint(tintR, tintB, tintG, tintA);
                    reloadTexture = true;
                    break;
                case THRESHOLD:
                    image.threshold(threshold);
                    reloadTexture = true;
                    break;
                case ADJUSTHSV:
                    image.adjustHSV(h, s, v);
                    reloadTexture = true;
                    break;
                case GAUSSIANBLUR:
                    image.gaussianBlur(kernel);
                    reloadTexture = true;
                    break;
                case BLOOM:
                    image.bloom(bloomThreshold, bloomKernel);
                    reloadTexture = true;
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

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Open Image")) {
                ImGui::InputText("File Name", readFilename, sizeof(readFilename));
                ImGui::SameLine();
                if (ImGui::Button("Open File")) {
                    editType = BUFFER;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

                

        static ImGuiViewport* viewport = ImGui::GetMainViewport();

        
        static ImGuiID dockspace_id = ImGui::GetID("MyDockspace");

        static bool firstLoop = true;
        if (firstLoop)
        {

            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);

            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
            //ImGui::DockBuilderSetNodePos(dockspace_id, viewport->Pos);

            ImGuiID leftSidebar, middleRight, middleWindow, rightSidebar, sequencerBar, imageViewport;

            ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, &leftSidebar, &middleRight);
            ImGui::DockBuilderSplitNode(middleRight, ImGuiDir_Left, 0.7f, &middleWindow, &rightSidebar);
            ImGui::DockBuilderSplitNode(middleWindow, ImGuiDir_Up, 0.8f, &imageViewport, &sequencerBar);
//            ImGuiID imageViewport = ImGui::DockBuilderSplitNode(middleWindow, ImGuiDir_Up, 0.6f, nullptr, &middleWindow);
//            ImGuiID sequencerBar = ImGui::DockBuilderSplitNode(middleWindow, ImGuiDir_Down, 0.4f, nullptr, &middleWindow);

            ImGui::DockBuilderDockWindow("LeftSidebar", leftSidebar);
            ImGui::DockBuilderDockWindow("RightSidebar", rightSidebar);
            ImGui::DockBuilderDockWindow("SequencerBar", sequencerBar);
            ImGui::DockBuilderDockWindow("ImageViewport", imageViewport);
            ImGui::DockBuilderFinish(dockspace_id);

            firstLoop = false;
        }

        ImGui::DockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_PassthruCentralNode);
        


        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);



        if (reloadTexture)
        {
            loadTexture(image, inputTexture);
            reloadTexture = false;
            textureLoaded = true;
            editType = NONE;
        }


        {
            ImGui::Begin("ImageViewport");
            ImVec2 windowSize = ImGui::GetContentRegionAvail();
            float windowAspectRatio = windowSize.x / windowSize.y;
            float imgHeight;
            float imgWidth;
            if (image.aspectRatio > windowAspectRatio)
            {
                // Image width = viewport width
                // Scale down height
                imgWidth = windowSize.x;
                imgHeight = windowSize.x / image.aspectRatio;
            }
            else
            {
                imgWidth = windowSize.y * image.aspectRatio;
                imgWidth = windowSize.y;
            }
                // Image height = viewport height
                // Scale down width
            if (textureLoaded)
            {
                ImGui::Image((void *)(intptr_t)inputTexture, ImVec2(imgWidth, imgHeight));
            }
            ImGui::End();
        }

        static IMGEditType editor = NONE;

        // Edit Options Bar - Left Sidebar
        ImGui::Begin("LeftSidebar");
        {
            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::Text("Application average %.ef ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

            ImGui::Text("Choose an edit to make:");
            ImGui::RadioButton("Make Greyscale", (int*)&editor, (int)TOGREYSCALE);
            ImGui::RadioButton("Make Negative", (int*)&editor, (int)TONEGATIVE);
            ImGui::RadioButton("Adjust Contrast", (int*)&editor, (int)SCALECONTRAST);
            ImGui::RadioButton("Adjust Tint", (int*)&editor, (int)COLORTINT);
            ImGui::RadioButton("Threshold Image", (int*)&editor, (int)THRESHOLD);
            ImGui::RadioButton("Adjust HSV", (int*)&editor, (int)ADJUSTHSV);
            ImGui::RadioButton("Gaussian Blur Image", (int*)&editor, (int)GAUSSIANBLUR);
            ImGui::RadioButton("Add Bloom", (int*)&editor, (int)BLOOM);

        }
        ImGui::End();

        // Editor Bar - Right Sidebar
        {
            ImGui::Begin("RightSidebar");
            switch (editor)
            {
                case TOGREYSCALE:
                    ImGui::Text("Choose different weights for converting to greyscale");

                    static bool lockR = false;
                    static bool lockG = false;
                    static bool lockB = false;
                    static float total = greyR + greyG + greyB;
                    if (ImGui::Checkbox("Lock R", &lockR))
                    {
                        lockG = false;
                        lockB = false;
                        if (total > 1.0f || total < 1.0f)
                        {
                            greyB = 1.0f - greyR - greyG;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Lock G", &lockG))
                    {
                        lockR = false;
                        lockB = false;
                    }
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Lock B", &lockB))
                    {
                        lockR = false;
                        lockG = false;
                    }

                    if (ImGui::VSliderFloat("r", ImVec2(10, 40), &greyR, 0.0f, 1.0f))
                    {
                        if (lockR || lockG) { greyB = 1.0f - greyR - greyG; }
                        else if (lockB) { greyG = 1.0f - greyR - greyB; }
                    }
                    ImGui::SameLine();
                    if (ImGui::VSliderFloat("g", ImVec2(10, 40), &greyG, 0.0f, 1.0f))
                    {
                        if (lockG || lockB) { greyR = 1.0f - greyB - greyG; }
                        else if (lockR) { greyB = 1.0f - greyR - greyG; }
                    }
                    ImGui::SameLine();
                    if (ImGui::VSliderFloat("b", ImVec2(10, 40), &greyB, 0.0f, 1.0f))
                    {
                        if (lockB || lockR) { greyG = 1.0f - greyR - greyB; }
                        else if (lockG) { greyR = 1.0f - greyB - greyG; }
                    }

                    break;
                case TONEGATIVE:
                    ImGui::Button("Negate Image");
                    break;
                case SCALECONTRAST:
                    ImGui::SliderFloat("Lower Bound", &contrastLower, -1.0f, 5.0f);
                    ImGui::SliderFloat("Upper Bound", &contrastUpper, 0.0f, 4.0f);
                    break;
                case COLORTINT:
                    ImGui::SliderFloat("R", &tintR, 0.0f, 1.0f);
                    ImGui::SliderFloat("G", &tintG, 0.0f, 1.0f);
                    ImGui::SliderFloat("B", &tintB, 0.0f, 1.0f);
                    ImGui::SliderFloat("A", &tintA, 0.0f, 1.0f);
                case THRESHOLD:
                    ImGui::SliderFloat("Threshold", &threshold, 0.0f, 1.0f);
                    break;
                case ADJUSTHSV:
                    ImGui::SliderFloat("Hue", &h, 0.0f, 360.0f);
                    ImGui::SliderFloat("Saturation", &s, 0.0f, 2.0f);
                    ImGui::SliderFloat("Value", &v, 0.0f, 1.0f);
                    break;
                case GAUSSIANBLUR:
                    ImGui::SliderInt("Kernel Size", &kernel, 0, 500);
                    break;
                case BLOOM:
                    ImGui::SliderInt("Bloom Kernel", &bloomKernel, 0, 500);
                    ImGui::SliderFloat("Bloom Threshold", &bloomThreshold, 0.0f, 1.0f);
                    break;
                default:
                    break;
            }
            ImGui::End();
        }


        {
            ImGui::Begin("SequencerBar");
            static bool toggleRealtime = false;
            ImGui::Checkbox("Toggle Realtime Edits", &toggleRealtime);
            if (toggleRealtime)
            {
                editType = editor;
            }
            if (ImGui::Button("Run"))
            {
                editType = editor;
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


        ImGui::EndFrame();
        // Start rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        GLFWwindow* backup = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup);
        glfwSwapBuffers(window);
    }

    // Cleanup
    if (inputTexture)
        glDeleteTextures(1, &inputTexture);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}
