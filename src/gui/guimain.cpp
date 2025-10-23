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
    GAUSSIANDEBLUR,
    BLOOM
} IMGEditType;

// Returns true if imageOutput was modified.
bool menu(IMGEditType shaderEffect, const Image& imageInput, Image& imageOutput)
{
    static ImagePipeline imagePipeline;
    switch (shaderEffect)
    {
        static bool run = false;
        static bool toggleRealtime = false;
        
        ImGui::Checkbox("Toggle Realtime, &toggleRealtime);
        run = ImGui::Button("Run") || toggleRealtime;

        case NONE:
            break;
        case TOGREYSCALE:
            static pixel4f_t grey = { 0.33f, 0.33f, 0.33f, 1.0f };
            ImGui::Text("Choose different weights for converting to greyscale");
            ImGui::VSliderFloat("r", ImVec2(30, 400), &grey.r, 0.0f, 1.0f);
            ImGui::SameLine();
            ImGui::VSliderFloat("g", ImVec2(30, 400), &grey.g, 0.0f, 1.0f);
            ImGui::SameLine();
            ImGui::VSliderFloat("b", ImVec2(30, 400), &grey.b, 0.0f, 1.0f);
            float total = grey.r + grey.g + grey.b;
            grey.r = grey.r / total;
            grey.g = grey.g / total;
            grey.b = grey.b / total;
            if (run)
                imagePipeline.toGreyscale(imageInput, imageOutput, grey);
            break;
        case TONEGATIVE:
            if (run)
                imagePipeline.toNegative(imageInput, imageOutput);
            break;
        case SCALECONTRAST:
            ImGui::SliderFloat("Contrast", &contrast, 0.0f, 5.0f);
            static float contrast = 1.0f;
            if (run)
                imagePipeline.scaleContrast(imageInput, imageOutput, contrast);
            break;
        case COLORTINT:
            static pixel4f_t tint = { 1.0f, 0.0f, 0.0f, 0.3f };
            ImGui::Text("Choose Tint Color"); 
            ImGui::SliderFloat("r", &tint.r, 0.0f, 1.0f);
            ImGui::SliderFloat("g", &tint.g, 0.0f, 1.0f);
            ImGui::SliderFloat("b", &tint.b, 0.0f, 1.0f);
            ImGui::SliderFloat("a", &tint.a, 0.0f, 1.0f);
            if (run)
                imagePipeline.colorTint(imageInput, imageOutput, tint);
            break;
        case THRESHOLD:
            static float threshold = 0.1f;
            ImGui::SliderFloat("Threshold", &threshold, 0.0f, 1.0f);
            if (run)
                imagePipeline.threshold(imageInput, imageOutput, threshold);
            break;
        case ADJUSTHSV:
            static pixel4f_hsv_t hsv = { 0.0f, 1.0f, 1.0f, 1.0f };
            ImGui::Text("Choose Tint Color"); 
            ImGui::SliderFloat("h", &hsv.h, 0.0f, 1.0f);
            ImGui::SliderFloat("s", &hsv.s, 0.0f, 1.0f);
            ImGui::SliderFloat("v", &hsv.v, 0.0f, 1.0f);
            if (run)
                imagePipeline.adjustHSV(imageInput, imageOutput, hsv);
            break;
        case GAUSSIANBLUR:
            static int gaussBlurKernel = 61;
            ImGui::Slider("Kernel Size", &gaussBlurKernel, 3, 100);
            if (run)
                imagePipeline.gaussianBlur(imageInput, imageOutput, gaussBlurKernel);
            break;
        case GAUSSIANDEBLUR:
            static int gaussDeBlurKernel = 61;
            ImGui::Slider("Kernel Size", &gaussDeBlurKernel, 3, 100);
            if (run)
                imagePipeline.gaussianDeBlur(imageInput, imageOutput, gaussDeBlurKernel);
            break;
        case BLOOM:
            static int bloomKernel = 61;
            static int bloomThreshold = 0.1f;
            static int bloomStrength = 0.2f;
            ImGui::SliderFloat("Bloom Threshold", &bloomThreshold, 0.0f, 1.0f);
            ImGui::Slider("Kernel Size", &bloomKernel, 3, 100);
            ImGui::SliderFloat("Bloom Strength", &bloomStrength, 0.0f, 1.0f);
            if (run)
                imagePipeline.bloom(imageInput, imageOutput, bloomThreshold, bloomKernel, bloomStrength);
            break;
        default:
            break;
    }
    return run;
}

static void glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void loadTexture(Image& image, GLuint& texture)
{
    if (texture)
        glDeleteTextures(1, &texture);
    glGenTextures(1, &texture);
    glBindTexture(1, texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_FLOAT, image.buffer);
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

    Image image;
    Image viewportImage;
    GLuint viewportTexture = 0;


    char readFilename[1024] = "";
    char writeFilename[1024] = "";
    bool reloadTexture = false;
    bool textureLoaded = false;
    bool updateComplete = false;

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
                    image.read(readFilename);
                    viewportImage.read(image);
                    reloadTexture = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Write Image")) {
                ImGui::InputText("File Output Name", writeFilename, sizeof(writeFilename));
                ImGui::SameLine();
                if (ImGui::Button("Write File"))
                {
                    viewportImage.write(writeFilename);
                }
                ImGui::EndMenu();
            }
            if (ImGui::Button("Apply Effect")) {
                image.read(viewportImage);
            }
            ImGui::EndMainMenuBar();
        }

        if (reloadTexture)
        {
            loadTexture(viewportImage, viewportTexture);
            reloadTexture = false;
            textureLoaded = true;
        }

                    textureLoaded = true;

        static ImGuiViewport* viewport = ImGui::GetMainViewport();
        static ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
        static bool firstLoop = true;
        if (firstLoop)
        {

            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);

            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            ImGuiID leftSidebar, middleRight, middleWindow, rightSidebar, sequencerBar, imageViewport;
            ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, &leftSidebar, &middleRight);
            ImGui::DockBuilderSplitNode(middleRight, ImGuiDir_Left, 0.7f, &middleWindow, &rightSidebar);
            ImGui::DockBuilderSplitNode(middleWindow, ImGuiDir_Up, 0.8f, &imageViewport, &sequencerBar);

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

        ImGui::Begin("ImageViewport");
        {
            ImVec2 windowSize = ImGui::GetContentRegionAvail();
            float windowAspectRatio = windowSize.x / windowSize.y;
            float imgHeight;
            float imgWidth;
            if (viewportImage.aspectRatio > windowAspectRatio)
            {
                // Image width = viewport width
                // Scale down height
                imgWidth = windowSize.x;
                imgHeight = windowSize.x / viewportImage.aspectRatio;
            }
            else
            {
                imgWidth = windowSize.y * viewportImage.aspectRatio;
                imgHeight = windowSize.y;
            }
                // Image height = viewport height
                // Scale down width
            if (textureLoaded)
            {
                ImGui::Image((void *)(intptr_t)viewportTexture, ImVec2(imgWidth, imgHeight));
            }
        }
        ImGui::End();

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
            ImGui::RadioButton("Gaussian DeBlur Image", (int*)&editor, (int)GAUSSIANDEBLUR);
            ImGui::RadioButton("Add Bloom", (int*)&editor, (int)BLOOM);

        }
        ImGui::End();

        // Editor Bar - Right Sidebar
        ImGui::Begin("RightSidebar");
        {
            reloadTexture = menu(editor, image, viewportImage);
        }
        ImGui::End();


        ImGui::Begin("SequencerBar");
        {
            static int startFrame = 1;
            static int endFrame = 120;
            static int transitionStartFrame = 1;
            static int transitionEndFrame = 120;
            static float transition;
            ImGui::
            ImGui::Slider("Start Frame", &startFrame, 0, 360);
            ImGui::Slider("End Frame", &endFrame, 0, 360);
            ImGui::Slider("Transition Start Frame", &transitionStartFrame, startFrame, endFrame);
            ImGui::Slider("Transition End Frame", &transitionEndFrame, startFrame, endFrame);
            static ImagePipeline imgPipeline;
            static Image imgIn;
            static Image imgOut;
            static Image mask;
            static int maskType = 0;
            ImGui::RadioButton("Horizontal Mask", &maskType, 0);
            ImGui::RadioButton("Vertical Mask", &maskType, 1);
            ImGui::RadioButton("Circle Mask", &maskType, 2);
            ImGui::RadioButton("Video Mask", &maskType, 3);
            static char[1024] readFilename1;
            static char[1024] readFilename2;
            static char[1024] writeFilename1;

            ImGui::InputText("File Input Base Name (Leave out 'xxxx.jpg')", readFilename1, sizeof(readFilename1));
            ImGui::InputText("File Input Base Name (Leave out 'xxxx.jpg')", readFilename2, sizeof(readFilename2));
            ImGui::InputText("File Output Base Name (Leave out 'xxxx.jpg')", writeFilename1, sizeof(writeFilename1));

            if (ImGui::Button("Run Edit Sequence"))
            {
                for (int currFrame = startFrame; currFrame <= endFrame; currFrame++)
                {
                    transition = float(currFrame) / float(transitionEndFrame - transitionStartFrame);

                    ImGui::TextBox("Current Frame: " + std::to_string(i));
                    std::string readName = readFilename1;
                    std::string readName2 = readFilename2;
                    std::string writeName = writeFilename1;
                    std::string suffix = "";
                    suffix
                    if (i < 10) { suffix += "000" + std::to_string(i) + ".jpg"; }
                    else if (i < 100) { suffix += "00" + std::to_string(i) + ".jpg"; }
                    else if (i < 1000) { suffix += "0" + std::to_string(i) + ".jpg"; }
                    readName += suffix;
                    writeName += suffix;

                    switch (maskType)
                    {
                        case 0:
                            imgIn.read(readName.c_str());
                            imgPipeline.horizontalMask(mask, imgIn.width, imgIn.height);
                            break;
                        case 1:
                            imgIn.read(readName.c_str());
                            imgPipeline.verticalMask(mask, imgIn.width, imgIn.height);
                            break;
                        case 2:
                            imgIn.read(readName.c_str());
                            imgPipeline.circleMask(mask, imgIn.width, imgIn.height);
                            break;
                        case 3:
                            imgIn.read(readName.c_str());
                            mask.read(readName2.c_str()); // Only handle the part that fits over imgIn for now.
                            break;
                        default:
                            break;
                    }
                    imgPipeline.composite(imgIn, imgOut, mask);
                    imgOut.write(writeName.c_str());
                }


                Image.read((std::string(readFilename) + 
                
                for (int i = 
                imgPipeline.horizontalWipe(mask, );




                reloadTexture = true;
            }

        }
        ImGui::End();

        ImGui::EndFrame();
        // Start rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
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
    if (viewportTexture)
        glDeleteTextures(1, &viewportTexture);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}

