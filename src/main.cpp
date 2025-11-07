#include "image.h"
#include "viewport.h"
#include <string>
#include <iostream>
#include <chrono>

void dvdLogoScene()
{
    Viewport viewport(1920, 1080);
    Image img;
    img.read("input/dvd-logo.png");
    vec2 scale;
    float rotation;
    vec2 translation;
    scale.x = 0.5f;
    scale.y = 0.5f;
    float scaledWidth = scale.x * float(img.width);
    float scaledHeight = scale.y * float(img.height);
    vec2 viewportTopLeft, viewportBottomRight;
    viewportTopLeft.x = 0.0f;//- float(viewport.viewport.width) / 2.0f;
    viewportTopLeft.y = 0.0f;//- float(viewport.viewport.height) / 2.0f;
    viewportBottomRight.x = float(viewport.viewport.width);// / 2.0f;
    viewportBottomRight.y = float(viewport.viewport.height);// / 2.0f;
    vec2 imgTopLeft, imgBottomRight;
    imgTopLeft.x = - scaledWidth / 2.0f;
    imgTopLeft.y = - scaledHeight / 2.0f;
    imgBottomRight.x = scaledWidth / 2.0f;
    imgBottomRight.y = scaledHeight / 2.0f;

    vec2 boundsTopLeft = viewportTopLeft + imgTopLeft;
    vec2 boundsBottomRight = viewportBottomRight + 3 * imgTopLeft;

    rotation = 0.0f;
    translation.x = 0.0f;
    translation.y = 0.0f;

    pixel4f_t clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    
    int frames[9] = { 0, 24, 30, 54, 60, 84, 90, 114, 120 };
    vec2 positions[9] =
    {
        boundsTopLeft + (vec2) { 600.0f, 0.0f },
        boundsBottomRight - (vec2) { 500.0f, 0.0f },
        boundsBottomRight - (vec2) { 0.0f, 400.0f },
        boundsTopLeft + (vec2) { 300.0f, 0.0f },
        boundsTopLeft + (vec2) { 0.0f, 200.0f },
        boundsBottomRight - (vec2) { 100.0f, 0.0f },
        boundsBottomRight - (vec2) { 0.0f, 100.0f },
        boundsTopLeft + (vec2) { 0.0f, 0.0f },
        boundsBottomRight + (vec2) { 100.0f, 0.0f }
    };
    for (int frame = 1; frame <= 120; frame++)
    {
        int startTime;
        int endTime;
        vec2 startPos;
        vec2 endPos;
        for (int i = 0; i + 1 < 9; i++)
        {
            if (frame > frames[i] && frame <= frames[i + 1])
            {
                startTime = frames[i];
                endTime = frames[i + 1];
                startPos = positions[i];
                endPos = positions[i + 1];
            }
        }

        float t = float(frame - startTime) / float(endTime - startTime);
        translation = mylerp(t, startPos, endPos);
        viewport.clearColor(clearColor);
        viewport.drawImage(img, scale, rotation, translation);
        std::string suffix = "";
        if (frame < 10)
            suffix = "000";
        else if (frame < 100)
            suffix = "00";
        else if (frame < 1000)
            suffix = "0";
        suffix += std::to_string(frame) + ".png";
        std::string outputFilename = "output/dvd-logo/dvd-logo" + suffix;
        viewport.viewport.write(outputFilename.c_str());
    }
}

void rotatingImageScene()
{

    Viewport viewport(1920, 1080);
    Image sky;
    sky.read("sky.jpg");
    Image sun;
    sun.read("sun.jpg");
    Image birds;
    birds.read("birds.jpg");

    vec2 skyScale = { 0.5f, 0.5f };
    float skyRotation = 0.0f;
    vec2 skyTranslation = { -1300.0f, -1000.0f };

    vec2 sunScale = { 0.5f, 0.5f };
    float sunRotation = 0.0f;
    vec2 sunTranslation = { 0.0f, 0.0f };

    vec2 birdsScale = { 0.3f, 0.3f };
    float birdsRotation = 0.0f;
    vec2 birdsTranslation = { - 1920.0f / 2.0f, 0.0f };

    pixel4f_t clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    
    for (int frame = 1; frame <= 120; frame++)
    {
        float t = float(frame) / 120.0f;

        sunRotation = mylerp(t, 0.0f, 360.0f * 5.0f);

        birdsTranslation.x = mylerp(t, -1920.0f / 2.0f, 1920.0f / 2.0f);
        birdsTranslation.y = 50.0f * sin(0.2f * birdsTranslation.x);

        viewport.clearColor(clearColor);
        viewport.drawImage(sky, skyScale, skyRotation, skyTranslation);
        viewport.drawImage(sun, sunScale, sunRotation, sunTranslation);
        viewport.drawImage(birds, birdsScale, birdsRotation, birdsTranslation);
        std::string suffix = "";
        if (frame < 10)
            suffix = "000";
        else if (frame < 100)
            suffix = "00";
        else if (frame < 1000)
            suffix = "0";
        suffix += std::to_string(frame) + ".png";
        std::string outputFilename = "output/rotating-img/rotating-img" + suffix;
        viewport.viewport.write(outputFilename.c_str());
    }
}

void spinningHeadlineScene()
{
    Viewport viewport(1920, 1080);
    Image newspaper;
    newspaper.read("spiderman.jpg");
    vec2 scale0 = { 0.0f, 0.0f };
    float rotation0 = 0.0f;
    vec2 translation0 = { 0.0f, 0.0f };

    vec2 scale1 = { 0.5f, 0.5f };
    float rotation1 = 7200.0f;
    vec2 translation1 = { 0.0f, -540.0f };

    pixel4f_t clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    
    for (int frame = 1; frame <= 120; frame++)
    {
        float t = float(frame) / 120.0f;

        vec2 scale = mylerp(t, scale0, scale1);
        float rotation = mylerp(t, rotation0, rotation1);
        vec2 translation = mylerp(t, translation0, translation1);

        viewport.clearColor(clearColor);
        viewport.drawImage(newspaper, scale, rotation, translation);
        std::string suffix = "";
        if (frame < 10)
            suffix = "000";
        else if (frame < 100)
            suffix = "00";
        else if (frame < 1000)
            suffix = "0";
        suffix += std::to_string(frame) + ".png";
        std::string outputFilename = "output/spinning/spinning" + suffix;
        viewport.viewport.write(outputFilename.c_str());
    }
}

void tileScene()
{
    Image output;
    Image input;
    Image upscaled;
    input.ensureBufferSize(540, 360);
    int widthRatio = 1920 / input.width;
    int heightRatio = 1080 / input.height;
    upscaled.ensureBufferSize(1920, 1080);
            
    output.ensureBufferSize(1920, 1080);
    int tileWidth = 1920 / 8;
    int tileHeight = 1080 / 8;
    for (int frame = 1; frame <= 120; frame++)
    {
        std::string suffix = "";
        if (frame < 10)
            suffix = "000";
        else if (frame < 100)
            suffix = "00";
        else if (frame < 1000)
            suffix = "0";
        suffix += std::to_string(frame) + ".png";
        std::string inputFilename = "input/sprite/sprite" + suffix;
        std::string outputFilename = "output/sprite/sprite" + suffix;

        input.read(inputFilename.c_str());
        for (int x = 0; x + 1 < input.width; x++)
        {
            for (int y = 0; y + 1 < input.height; y++)
            {
                rgba_quad_t rgbaQuad = { .topLeft = input(x, y),
                                      .topRight = input(x + 1, y),
                                      .bottomLeft = input(x, y + 1),
                                      .bottomRight = input(x + 1, y + 1) };
                for (int xx = 0; xx < widthRatio; xx++)
                {
                    for (int yy = 0; yy < heightRatio; yy++)
                    {
                        float tx = float(xx) / float(widthRatio - 1);
                        float ty = float(yy) / float(heightRatio - 1);
                        int xIndex = x * widthRatio + xx;
                        int yIndex = y * widthRatio + yy;
                        upscaled(xIndex, yIndex) = bilinear_interpolation(tx, ty, rgbaQuad);
                    }
                }
            }
        }

        for (int x = 0; x < tileWidth; x++)
        {
            for (int y = 0; y < tileHeight; y++)
            {
                for (int xx = 0; xx < 8; xx++)
                {
                    for (int yy = 0; yy < 8; yy++)
                    {
                        int xIndex = xx * tileWidth + x;
                        int yIndex = yy * tileHeight + y;
                        output(xIndex, yIndex) = upscaled(x * 8, y * 8);
                    }
                }
            }
        }
        output.write(outputFilename.c_str());
    }
}


void pixelatedScene()
{
    Image output;
    Image input;
    Image upscaled;
    input.ensureBufferSize(540, 360);
    int widthRatio = 1920 / input.width;
    int heightRatio = 1080 / input.height;
    upscaled.ensureBufferSize(1920, 1080);
            
    output.ensureBufferSize(1920, 1080);
    int tileWidth = 32;
    int tileHeight = 32;
    for (int frame = 1; frame <= 120; frame++)
    {
        std::string suffix = "";
        if (frame < 10)
            suffix = "000";
        else if (frame < 100)
            suffix = "00";
        else if (frame < 1000)
            suffix = "0";
        suffix += std::to_string(frame) + ".png";
        std::string inputFilename = "input/sprite/sprite" + suffix;
        std::string outputFilename = "output/pixelated/pixelated" + suffix;

        input.read(inputFilename.c_str());
        for (int x = 0; x + 1 < input.width; x++)
        {
            for (int y = 0; y + 1 < input.height; y++)
            {
                rgba_quad_t rgbaQuad = { .topLeft = input(x, y),
                                      .topRight = input(x + 1, y),
                                      .bottomLeft = input(x, y + 1),
                                      .bottomRight = input(x + 1, y + 1) };
                for (int xx = 0; xx < widthRatio; xx++)
                {
                    for (int yy = 0; yy < heightRatio; yy++)
                    {
                        float tx = float(xx) / float(widthRatio - 1);
                        float ty = float(yy) / float(heightRatio - 1);
                        int xIndex = x * widthRatio + xx;
                        int yIndex = y * widthRatio + yy;
                        upscaled(xIndex, yIndex) = bilinear_interpolation(tx, ty, rgbaQuad);
                    }
                }
            }
        }

        for (int x = 0; x < 1920 / 32; x++)
        {
            for (int y = 0; y < 1080 / 32; y++)
            {
                pixel4f_t tileColor = { 0.0f, 0.0f, 0.0f, 1.0f };
                for (int xx = 0; xx < 32; xx++)
                {
                    for (int yy = 0; yy < 32; yy++)
                    {

                        int xIndex = x * tileWidth + xx;
                        int yIndex = y * tileHeight + yy;
                        tileColor = tileColor + upscaled(xIndex, yIndex);
                    }
                }
                tileColor = tileColor / float(32 * 32);
                for (int xx = 0; xx < 32; xx++)
                {
                    for (int yy = 0; yy < 32; yy++)
                    {

                        int xIndex = x * tileWidth + xx;
                        int yIndex = y * tileHeight + yy;
                        output(xIndex, yIndex) = tileColor;
                    }
                }
            }
        }
        output.write(outputFilename.c_str());
    }
}

int main()
{
    //dvdLogoScene();
    //rotatingImageScene();
    //spinningHeadlineScene();
    //tileScene();
    pixelatedScene();

    /*
    ImagePipeline imgPipeline;
    Image img;
    Image imgblurred;
    Image imgout;
    Image thresh;
    img.read("dvd-logo.png");
    Viewport viewport(img.width, img.height);
    vec2 scale;
    float rotation;
    vec2 translation;
    scale.x = 0.2f;
    scale.y = 15.0f;
    rotation = 0.0f;
    translation.x = 100.0f;
    translation.y = 0.0f;
    viewport.drawImage(img, scale, rotation, translation);
    viewport.viewport.write("scaledSunset.jpg");
    */

    /*
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
    */

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
