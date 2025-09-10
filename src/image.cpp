/********************************************
 * Author: Kyle Bueche
 * File: image.cpp
 *
 *******************************************/

#include "image.h"

ImageI::ImageI(uint32_t width, uint32_t height)
{
    width = width;
    height = height;
    aspectRatio = float(width) / float(height);
    pixels = std::vector<PixelI>(width * height);
}

ImageF::ImageF(uint32_t width, uint32_t height)
{
    width = width;
    height = height;
    aspectRatio = float(width) / float(height);
    pixels = std::vector<PixelF>(width * height);
}

/********************************************************
 * This is kind of terrible but its okay.
 *
 * First you load the entire image to the data variable.
 * Then you copy the entire thing to an std::vector.
 * Then you return the image class, which creates a new
 * std::vector, copying the image yet again.
 *
 * The full image is copied twice, this would be terrible
 * for loading a large sequence of images.
 *
 * Do I need an std::vector? I don't need dynamically
 * resizeable arrays. Cropping may not even benefit from
 * a vector.
 ********************************************************/
ImageI loadImageI(const char *path)
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t nrChannels = 0;
    unsigned char *data = stbi_load(texturePath, &width, &height, &nrChannels, 0);

    ImageI image(width, height);
    if (data)
    {
        int dataIndex;
        if (nrChannels == 3)
        {
            for (int i = 0; i < width * height; i++)
            {
                dataIndex = i * 3;
                image.pixels[i].r = data[dataIndex]
                image.pixels[i].r = data[dataIndex + 1]
                image.pixels[i].r = data[dataIndex + 2]
                image.pixels[i].a = 0;
            }
        }
        if (nrChannels == 4)
        {
            for (int i = 0; i < width * height; i++)
            {
                dataIndex = i * 4;
                image.pixels[i].r = data[dataIndex]
                image.pixels[i].r = data[dataIndex + 1]
                image.pixels[i].r = data[dataIndex + 2]
                image.pixels[i].a = data[dataIndex + 3];
            }
        }
    }
    else
    {
        std::cout << "ERROR: Failed to load image" << std::endl;
    }
    return image;
}

ImageI toImageI(ImageF& srcImg)
{
    ImageI image(srcImg.width, srcImg.height);
    for (int i = 0; i < srcImg.pixels.size(); i++)
    {
        image.pixels[i].r = int(255.99f * srcImg.pixels[i].r);
        image.pixels[i].g = int(255.99f * srcImg.pixels[i].g);
        image.pixels[i].b = int(255.99f * srcImg.pixels[i].b);
        image.pixels[i].a = int(255.99f * srcImg.pixels[i].a);
    }
    return image;
}

ImageF toImageF(ImageI& srcImg)
{
    ImageF image(srcImg.width, srcImg.height);
    float scaler = 1.0f / 255.0f;
    for (int i = 0; i < srcImg,pixels.size(); i++)
    {
        image.pixels[i].r = scalar * float(srcImg.pixels[i].r);
        image.pixels[i].g = scalar * float(srcImg.pixels[i].g);
        image.pixels[i].b = scalar * float(srcImg.pixels[i].b);
        image.pixels[i].a = scalar * float(srcImg.pixels[i].a);
    }
    return image;
}
