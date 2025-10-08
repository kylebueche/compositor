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

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} pixel4i_t;


typedef struct
{
    float r;
    float g;
    float b;
    float a;
} pixel4f_t;

typedef struct
{
    float h;
    float s;
    float v;
    float a;
} pixel4f_hsv_t;

pixel4f_hsv_t pixelRGBAtoHSVA(const pixel4f_t& pixel);
pixel4f_t pixelHSVAtoRGBA(const pixel4f_hsv_t& pixel);

class ImageHSV;

class ImageF
{
    public:
        unsigned int width;
        unsigned int height;
        unsigned int pixelCount;
        float aspectRatio;
        pixel4f_t *pixels;
        pixel4f_t *temp;
        pixel4f_t *output;

        ImageF();
        ~ImageF();
        void ensureBuffersNull();
        void ensureBuffersDeleted();
        void ensureBufferSize(int width, int height);
        const bool null() const;
        void buffer(const char* filename); // Load image from file
        void buffer(const ImageF& image); // Copy RGBA image to RGBA image
        //void buffer(const ImageHSV&); // Convert HSVA image to RGBA image
        //void bufferTemp(const ImageHSV&); // Buffer from temp to temp instead
        void write(const char* filename); // Write image to file
        void apply(); // Swap ptrs of pixels & temp to apply changes
        void toGreyscale(float rWeight, float gWeight, float bWeight);
        void toNegative();
        pixel4f_t max();
        pixel4f_t min();
        void scaleContrast(float lowerBound, float upperBound);
        void colorTint(float r, float g, float b, float a);
        void threshold(float a);
        void thresholdColor(float a, float strength);
        void adjustHSV(float hueShift, float saturationScale, float valueScale);
        void rotateHue(float shift);
        void scaleSaturation(float scale);
        void scaleValue(float scale);
        void gaussianBlur(int kernel);
        void blendForeground(const ImageF& image);
        void add(const ImageF& image);
        void bloom(float threshold, int kernel);
        inline int index(int x, int y)
        {
            // Mirror index along edges if out of bounds
            if (x < 0)
            {
                x = - x;
            }
            else if (x > width - 1)
            {
                x = width - 1 - (x - width);
            }
            if (y < 0)
            {
                y = - y;
            }
            else if (y > height - 1)
            {
                y = height - 1 - (y - height);
            }
            return y * width + x;
        }
};

//class ImageHSV
//{
//    public:
//        unsigned int width;
//        unsigned int height;
//        unsigned int pixelCount;
//        float aspectRatio;
//        pixel4f_hsv_t *pixels; // Read and write to
//        pixel4f_hsv_t *temp; // Image processing functions write here before apply() swaps ptrs.
//
//        ImageHSV();
//        ~ImageHSV();
//        void ensureBuffersNull();
//        void ensureBuffersDeleted();
//        void ensureBufferSize(int, int);
//        bool null();
//        void buffer(const ImageHSV&); // Copy HSVA image to HSVA image
//        void buffer(const ImageF&); // Convert RGBA image to HSVA image
//        void apply(); // Swap ptrs of pixels & temp to apply changes
//        void scaleSaturation(float);
//        void scaleValue(float);
//        void rotateHue(float);
//};

#endif
