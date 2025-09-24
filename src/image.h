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

pixel4f_hsv_t pixelRGBAtoHSVA(const pixel4f_t&);
pixel4f_t pixelHSVAtoRGBA(const pixel4f_hsv_t&);

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

        ImageF();
        ~ImageF();
        void ensureBuffersNull();
        void ensureBuffersDeleted();
        void ensureBufferSize(int, int);
        bool null();
        void buffer(const char*); // Load image from file
        void buffer(const ImageF&); // Copy RGBA image to RGBA image
        //void buffer(const ImageHSV&); // Convert HSVA image to RGBA image
        //void bufferTemp(const ImageHSV&); // Buffer from temp to temp instead
        void write(const char*); // Write image to file
        void apply(); // Swap ptrs of pixels & temp to apply changes
        void toGreyscale(float, float, float);
        void toNegative();
        void scaleContrast(float);
        void colorTint(float, float, float, float);
        void threshold(float);
        void adjustHSV(float, float, float);
        void rotateHue(float);
        void scaleSaturation(float);
        void scaleValue(float);
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
