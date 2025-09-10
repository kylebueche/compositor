/********************************************
 * Author: Kyle Bueche
 * File: image.h
 *
 *******************************************/

#ifndef IMAGE_H
#define IMAGE_H

#include <cstring>

struct SIMDImageSequence
{
    // List of pixel locations for the first image in the sequence
    char **r;
    char **g;
    char **b;
    char **a;
    // Glob of data structured like:
    //  pixel[i][j]
    //    -> image[k]
    char *rawR;
    char *rawG;
    char *rawB;
    char *rawA;


struct PixelI
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} PixelI;


struct PixelF
{
    float r;
    float g;
    float b;
    float a;
} PixelF;

class ImageI
{
    public:
        uint32_t width;
        uint32_t height;
        float aspectRatio;
        std::vector<PixelI> pixels;

        ImageI(uint32_t width, uint32_t height);
        // Not bounds checked. Use responsibly.
        inline unsigned int index(unsigned int x, unsigned int y)
        {
            return y * width + x;
        }
};

class ImageF
{
    public:
        uint32_t width;
        uint32_t height;
        float aspectRatio;
        std::vector<PixelF> pixels;

        ImageF(uint32_t width, uint32_t height);
        // Not bounds checked. Use responsibly.
        inline unsigned int index(unsigned int x, unsigned int y)
        {
            return y * width + x;
        }
};

ImageI loadImageI(const char *path);
ImageI toImageI(ImageF& srcImg);
ImageF toImageF(ImageI& srcImg);

#endif
