/************************************************************************
 * Author: Kyle Bueche
 * File: image.h
 *
 * Header with some inline functions for RGBA and HSVA operations.
 * Pixel functions are used in a greater ImagePipeline class.
************************************************************************/

#ifndef IMAGE_H
#define IMAGE_H

#include <cstring>
#include <cstdint>
#include <iostream>
#include <cmath>
#include <numbers>
#include <vector>

#include "color.h"

const int NUM_CHANNELS = 4;

std::string fileName(std::string stem, int frame, std::string extension);

// Handles file I/O, dynamic sizing, single-image storing.
class Image
{
public:
    std::vector<col4f> buffer;
    int width;
    int height;
    float aspectRatio;
    
    int pixelCount; // Current image size
    
    Image();
    Image(int width, int height);
    //~Image();
    //const bool null() const; // Check if buffer is nullptr
    void resize(int width, int height);
    void read(const char* filename); // Load image from file
    void read(const Image& image); // Copy image from other image
    void write(const char* filename); // Write image to file
    // For the following: 0 <= tx <= width - 1, 0 <= ty <= height - 1
    col4f nearestNeighbor(float tx, float ty);
    col4f bilinearInterpolation(float tx, float ty);
    col4f bicubicInterpolation(float tx, float ty);

    void clearColor(col4f color)
    {
        for (int i = 0; i < pixelCount; i++)
        {
            buffer[i] = color;
        }
    }

    // Make myImage(i) valid:
    inline col4f& operator[](size_t i) noexcept {
        return buffer[i];
    }
    inline const col4f& operator[](size_t i) const noexcept {
        return buffer[i];
    }

    // Make myImage(x, y) valid
    inline col4f& operator()(size_t x, size_t y) noexcept {
        return buffer[y * width + x];
    }
    inline const col4f& operator()(size_t x, size_t y) const noexcept {
        return buffer[y * width + x];
    }

    // Safe 2d index with nearest-neighbor behavior when out of bounds
    inline col4f& clamped(size_t x, size_t y) noexcept {
        int inboundsX = clamp(x, 0, width - 1);
        int inboundsY = clamp(y, 0, height - 1);
        return buffer[inboundsY * width + inboundsX];
    }
    inline const col4f& clamped(size_t x, size_t y) const noexcept {
        int inboundsX = clamp(x, 0, width - 1);
        int inboundsY = clamp(y, 0, height - 1);
        return buffer[inboundsY * width + inboundsX];
    }

    /*
    // Safe 2d index with reflection behavior when out of bounds
    inline col4f& reflected(size_t x, size_t y) noexcept {
        int inboundsX = std::abs((x % (2 * width - 2) + (2 * width - 2)) % (2 * width - 2));
        inboundsX = (x < width) ? x : (2 * width - 2) - x;
        int inboundsY = std::abs((y % (2 * height - 2) + (2 * height - 2)) % (2 * height - 2));
        inboundsY = (y < height) ? y : (2 * height - 2) - y;
        return buffer[inboundsY * width + inboundsX];
    }
    inline const col4f& reflected(size_t x, size_t y) const noexcept {
        int inboundsX = std::abs((x % (2 * width - 2) + (2 * width - 2)) % (2 * width - 2));
        inboundsX = (x < width) ? x : (2 * width - 2) - x;
        int inboundsY = std::abs((y % (2 * height - 2) + (2 * height - 2)) % (2 * height - 2));
        inboundsY = (y < height) ? y : (2 * height - 2) - y;
        return buffer[inboundsY * width + inboundsX];
    }
    */
};

// Handles operations that require a memory pool.
class ImagePipeline
{
public:
    // Workspace
    Image temp1;
    Image temp2;
    Image temp3;

    // 1 Image input, non-Image output
    col4f max(const Image& image);
    col4f min(const Image& image);

    // 1 Image input, 1 Image output
    // Ensure output fits input size
    void toNegative(const Image& in, Image& out);
    void scaleContrast(const Image& in, Image& out, float contrast);
    void scaleBrightness(const Image& in, Image& out, float brightness);
    void scaleTransparency(const Image& in, Image& out, float transparency);
    void toGreyscale(const Image& in, Image& out, col4f weights);
    void colorTint(const Image& in, Image& out, col4f tint);
    void threshold(const Image& in, Image& out, float threshold);
    void thresholdColor(const Image& in, Image& out, float threshold);
    void adjustHSV(const Image& in, Image& out, col4f_hsv_t hsv);
    void gaussianBlur(const Image& in, Image& out, int kernel);
    void gaussianDeBlur(const Image& in, Image& out, int kernel);
    void bloom(const Image& in, Image& out, float threshold, int kernel, float strength);

    // 2 Image input, 1 Image output
    // Ensure output fits the larger width and larger height from each image
    void blendForeground(const Image& fg, const Image& bg, Image& out);
    void add(const Image& in1, const Image& in2, Image& out);
    void subtract(const Image& in1, const Image& in2, Image& out);

    // Mask output
    void maskify(const Image& imgIn, Image& maskOut);
    void horizontalMask(Image& maskOut, float t, int feathering, int width, int height);
    void verticalMask(Image& maskOut, float t, int feathering, int width, int height);
    void circleMask(Image& maskOut, float t, int feathering, int width, int height);
    void perlinNoiseMask(Image& maskOut, float frequency, float z, int width, int height);
    
    // 2 Image input, 1 Mask input, 1 Image output
    // Not size checked for now
    void composite(const Image& imgIn1, const Image& imgIn2, Image& imgOut, const Image& mask);
};



#endif
