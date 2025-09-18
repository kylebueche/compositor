/********************************************
 * Author: Kyle Bueche
 * File: image.h
 *
 *******************************************/

#ifndef IMAGE_H
#define IMAGE_H

#include <cstring>
#include <cstdint>
#include <iostream>

const int NUM_CHANNELS = 4;

typedef union
{
    struct
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    } col;
    uint8_t array[4];
} pixel4i_t;


typedef union
{
    struct
    {
        float r;
        float g;
        float b;
        float a;
    } col;
    float array[4];
} pixel4f_t;

typedef union
{
    struct
    {
        float h;
        float s;
        float v;
        float a;
    } col;
    float array[4];
} pixel4f_hsv_t;

class ImageF
{
    public:
        unsigned int width;
        unsigned int height;
        unsigned int pixelCount;
        float aspectRatio;
        pixel4f_t *pixels;

        ImageF(const char*);
        ImageF(const ImageF&);
        ~ImageF();
        buffer(const char*);
        buffer(const ImageF&);
        void toGreyscale(float, float, float);
        void toNegative();
        inline unsigned int index(unsigned int x, unsigned int y)
        {
            return y * width + x;
        }
};

class ImageHSV
{
    public:
        unsigned int width;
        unsigned int height;
        unsigned int pixelCount;
        float aspectRatio;
        pixel4f_hsv_t *pixels;
        ImageHSV(const ImageHSV&);
        ImageHSV(const ImageF&);
        ImageHSV(const ImageI&);
        ~ImageHSV();
        inline unsigned int index(unsigned int x, unsigned int y)
        {
            return y * width + x;
        }
};

#endif
