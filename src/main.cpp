#include "image.h"
#include <string>
#include <iostream>
#include <chrono>

int main()
{
    ImagePipeline imgPipeline;
    Image img;
    Image imgblurred;
    Image imgout;
    Image thresh;
    img.read("input/sunset.jpg");
    imgPipeline.gaussianBlur(img, imgblurred, 9);
    imgPipeline.gaussianDeBlur(imgblurred, imgout, 9);
    imgblurred.write("sunsetblurred.jpg");
    imgout.write("sunsetdeblurred.jpg");
    imgPipeline.subtract(img, imgout, thresh);
    thresh.write("sub.jpg");
    pixel4f_t a = imgPipeline.min(thresh);
    std::cout << "r: " << a.r << ", g: " << a.g << ", b: " << a.b << ", a: " << a.a << std::endl;
    imgPipeline.scaleBrightness(thresh, thresh, 10.0f);
    thresh.write("thresh.jpg");

    /*
    int vidFrames = 120;
    ImagePipeline imgPipeline;
    Image imgIn1;
    Image imgIn2;
    Image imgOut;
    Image mask;
    int maskType = 0;
    std::string readFilename1;
    std::string readFilename2;
    std::string readFilename3;
    std::string writeFilename;
    std::cout << "First Video File Stem to Read (Leave out '****.jpg'): ";
    std::cin >> readFilename1;
    std::cout << "Second Video File Stem to Read (Leave out '****.jpg'): ";
    std::cin >> readFilename2;
    std::cout << "Video Mask to Read (Leave blank if none): ";
    std::cin >> readFilename3;
    std::cout << "File Stem to Write: ";
    std::cin >> writeFilename;
    std::cout << "Video Frame Count: ";
    std::cin >> vidFrames;
    std::cout << "Mask Type (0=horizontal, 1=vertical, 2=circle, 3=video): ";
    std::cin >> maskType;

    auto totalStart = std::chrono::steady_clock::now();
    auto timeSpentNotPrinting = static_cast<std::chrono::milliseconds>(0);

    for (int currFrame = 1; currFrame <= vidFrames; currFrame++)
    {
        std::cout << "Starting Frame " << currFrame << "!" << std::endl;
        auto start = std::chrono::steady_clock::now();
        float transition = float(currFrame) / float(vidFrames);

        std::string suffix = "";
        if (currFrame < 10) {
            suffix = "000";
        }
        else if (currFrame < 100) {
            suffix = "00";
        }
        else if (currFrame < 1000) {
            suffix = "0";
        }
        suffix += std::to_string(currFrame) + ".jpg";
        std::string readFile1 = "input/" + readFilename1 + suffix;
        std::string readFile2 = "input/" + readFilename2 + suffix;
        std::string readFile3 = "input/" + readFilename3 + suffix;
        std::string writeFile = "output/" + writeFilename + suffix;

        switch (maskType)
        {
            case 0:
                imgIn1.read(readFile1.c_str());
                imgIn2.read(readFile2.c_str());
                imgPipeline.horizontalMask(mask, transition, 30, imgIn1.width, imgIn1.height);
                break;
            case 1:
                imgIn1.read(readFile1.c_str());
                imgIn2.read(readFile2.c_str());
                imgPipeline.verticalMask(mask, transition, 30, imgIn1.width, imgIn1.height);
                break;
            case 2:
                imgIn1.read(readFile1.c_str());
                imgIn2.read(readFile2.c_str());
                imgPipeline.circleMask(mask, transition, 30, imgIn1.width, imgIn1.height);
                break;
            case 3:
                imgIn1.read(readFile1.c_str());
                imgIn2.read(readFile2.c_str());
                mask.read(readFile3.c_str()); // Only handle the part that fits over imgIn for now.
                imgPipeline.maskify(mask, mask);
                break;
            default:
                break;
        }
        imgPipeline.composite(imgIn1, imgIn2, imgOut, mask);
        imgOut.write(writeFile.c_str());
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        timeSpentNotPrinting += duration;
        std::cout << " - Completed in " << duration << "ms!" << std::endl;
    }

    auto totalEnd = std::chrono::steady_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(totalEnd - totalStart);
    auto timeSpentPrinting = totalDuration - timeSpentNotPrinting;
    std::cout << std::endl << "Sequence Complete!" << std::endl;
    std::cout << "Total Time Taken: " << totalDuration << std::endl;
    std::cout << "Total Time Spent Doing Necessary Operations: " << timeSpentNotPrinting << std::endl;
    std::cout << "Total Time Spent Printing to Terminal: " << timeSpentPrinting << std::endl;
*/
    return 0;
}
