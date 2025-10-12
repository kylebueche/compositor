#include "image.h"
#include <string>
#include <iostream>
#include <chrono>

float lerp(float t, float t0, float t1)
{
    return t * (t1 - t0) + t0;
}

int main()
{
    std::string inputName = "sunset.jpg";
    std::string outputName = "Movie";

    ImageF image;
    image.buffer(inputName.c_str());
    ImageF image2;
    std::string frameNum = "";

//    for (int i = 0; i < 120; i++)
//    {
//        auto start = std::chrono::steady_clock::now();
//        std::cout << "Starting Frame " << i << "...";
//        if (i < 10)
//            frameNum = "000" + std::to_string(i);
//        else if (i < 100)
//            frameNum = "00" + std::to_string(i);
//        else if (i < 1000)
//            frameNum = "0" + std::to_string(i);
//            
//        float threshold = lerp((float(i + 1) / 120), 0.75, 0.25);
//        image.threshold(threshold);
//        image.write((outputName + frameNum + ".jpg").c_str());
//        auto end = std::chrono::steady_clock::now();
//        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//        std::cout << " ...Complete in " << duration << "!" << std::endl;
//    }
//        
//    for (int i = 120; i < 240; i++)
//    {
//        auto start = std::chrono::steady_clock::now();
//        std::cout << "Starting Frame " << i << "...";
//        if (i < 10)
//            frameNum = "000" + std::to_string(i);
//        else if (i < 100)
//            frameNum = "00" + std::to_string(i);
//        else if (i < 1000)
//            frameNum = "0" + std::to_string(i);
//            
//        float threshold = lerp((float(i + 1 - 120) / 120), 0.75, 0.25);
//        image2.buffer(image);
//        image2.threshold(threshold);
//        image2.apply();
//        image2.gaussianBlur(61);
//        image2.write((outputName + frameNum + ".jpg").c_str());
//        auto end = std::chrono::steady_clock::now();
//        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//        std::cout << " ...Complete in " << duration << "!" << std::endl;
//    }

    for (int i = 240; i < 360; i++)
    {
        auto start = std::chrono::steady_clock::now();
        std::cout << "Starting Frame " << i << "...";
        if (i < 10)
            frameNum = "000" + std::to_string(i);
        else if (i < 100)
            frameNum = "00" + std::to_string(i);
        else if (i < 1000)
            frameNum = "0" + std::to_string(i);
            
        float threshold = lerp((float(i + 1 - 240) / 120), 0.75, 0.25);
        image.bloom(threshold, 61);
        image.write((outputName + frameNum + ".jpg").c_str());
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << " ...Complete in " << duration << "!" << std::endl;
    }
    
    return 0;
}
