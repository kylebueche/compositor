/********************************************
 * Author: Kyle Bueche
 * File: image.cpp
 *
 *******************************************/

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
#include <cmath>
#include <numbers>
#include <vector>
#include <new>
#include <iomanip>
#include "image.h"
#include "matrix.h"

Image::Image()
{
    this->buffer = nullptr;
    this->width = 0;
    this->height = 0;
    this->aspectRatio = 1.0f;
    this->pixelCount = 0;
    this->bufferSize = 0;
}

Image::~Image()
{
    ensureBufferDeleted();
}

void Image::ensureBufferDeleted()
{
    if (this->buffer != nullptr)
        delete[] this->buffer;
    this->buffer = nullptr;
    this->width = 0;
    this->height = 0;
    this->aspectRatio = 1.0f;
    this->pixelCount = 0;
    this->bufferSize = 0;
}

void Image::ensureBufferSize(int width, int height)
{
    if (width >= 0 && height >= 0)
    {
        int newPixelCount = width * height;
        // Reallocate buffer if not big enough
        if (newPixelCount > this->bufferSize)
        {
            ensureBufferDeleted();
            this->buffer = new pixel4f_t[newPixelCount];
            // Handle allocation failure
if (this->buffer == nullptr)
            {
                std::cerr << "Error: buffer allocation failed in buffer " << this << std::endl;
                return;
            }
            this->bufferSize = newPixelCount;
        }

        this->width = width;
        this->height = height;
        this->pixelCount = newPixelCount;
        this->aspectRatio = float(width) / float(height);
    }
}

const bool Image::null() const
{
    bool bufferNull = (buffer == nullptr);
    if (bufferNull)
    {
        std::cerr << "Image buffer loading failed." << std::endl;
    }
    return bufferNull;
}

// From file bufferer
void Image::read(const char *filename)
{
    int newWidth;
    int newHeight;
    int nrChannels;

    // Desired channels is 4, will always convert to an rgba image.
    unsigned char *data = stbi_load(filename, &newWidth, &newHeight, &nrChannels, NUM_CHANNELS);

    // Handle stbi loading errors
    if (data)
    {
        ensureBufferSize(newWidth, newHeight);
        pixel4i_t* intBuffer = (pixel4i_t*) data;
        for (int i = 0; i < pixelCount; i++)
        {
            buffer[i] = pixelItoF(intBuffer[i]);
        }
        stbi_image_free(data);
    }
    else
    {
        std::cerr << "ERROR: STBI Failed to load the image" << std::endl;
    }
}

void Image::read(const Image& image)
{
    ensureBufferSize(image.width, image.height);
    for (int i = 0; i < image.pixelCount; i++)
    {
        buffer[i] = image[i];
    }
}

// TODO: Handle intBuffer allocation failure
void Image::write(const char *filename)
{
    pixel4i_t *intBuffer = new pixel4i_t[pixelCount];
    if (intBuffer != nullptr)
    {
        for (int i = 0; i < pixelCount; i++)
        {
            intBuffer[i] = pixelFtoI(buffer[i]);
        }
        stbi_write_png(filename, this->width, this->height, NUM_CHANNELS, intBuffer, this->width * sizeof(uint8_t) * NUM_CHANNELS);
        delete[] intBuffer;
    }
    else
    {
        std::cerr << "ERROR: Intbuffer allocation failed while writing to file." << std::endl;
    }
}

// Consider making these Image:: member functions
// 1 Image input, non-Image output
pixel4f_t ImagePipeline::max(const Image& image)
{
    pixel4f_t max;
    max.r = - std::numeric_limits<float>::infinity();
    max.g = - std::numeric_limits<float>::infinity();
    max.b = - std::numeric_limits<float>::infinity();
    max.a = - std::numeric_limits<float>::infinity();

    for (int i = 0; i < image.pixelCount; i++)
    {
        max.r = std::max(image[i].r, max.r);
        max.g = std::max(image[i].g, max.g);
        max.b = std::max(image[i].b, max.b);
        max.a = std::max(image[i].a, max.a);
    }

    return max;
}

pixel4f_t ImagePipeline::min(const Image& image)
{
    pixel4f_t min;
    min.r = std::numeric_limits<float>::infinity();
    min.g = std::numeric_limits<float>::infinity();
    min.b = std::numeric_limits<float>::infinity();
    min.a = std::numeric_limits<float>::infinity();

    for (int i = 0; i < image.pixelCount; i++)
    {
        min.r = std::min(image[i].r, min.r);
        min.g = std::min(image[i].g, min.g);
        min.b = std::min(image[i].b, min.b);
        min.a = std::min(image[i].a, min.a);
    }

    return min;
}

// 1 Image input, 1 Image output
// Ensure output fits input size

void ImagePipeline::toNegative(const Image& input, Image& output)
{
    output.ensureBufferSize(input.width, input.height);
    for (int i = 0; i < input.pixelCount; i++)
    {
        output[i] = negative(input[i]);
    }
}


void ImagePipeline::scaleContrast(const Image& input, Image& output, float contrast)
{
    output.ensureBufferSize(input.width, input.height);
    float higherBound = contrast;
    float lowerBound = 1.0f / contrast;
    float deltaOut = higherBound - lowerBound;
    pixel4f_t minPixel = min(input);
    pixel4f_t maxPixel = max(input);
    float min = std::min(std::min(minPixel.r, minPixel.g), minPixel.b);
    float max = std::max(std::max(maxPixel.r, maxPixel.g), maxPixel.b);
    float delta = max - min;
    
    for (int i = 0; i < input.pixelCount; i++)
    {
        output[i].r = (((input[i].r - min) / delta) * deltaOut) + lowerBound;
        output[i].g = (((input[i].g - min) / delta) * deltaOut) + lowerBound;
        output[i].b = (((input[i].b - min) / delta) * deltaOut) + lowerBound;
        output[i].a = input[i].a;
    }
}

void ImagePipeline::scaleBrightness(const Image& input, Image& output, float scale)
{
    output.ensureBufferSize(input.width, input.height);
    for (int i = 0; i < input.pixelCount; i++)
    {
        output[i] = scale * input[i];
    }
}

void ImagePipeline::toGreyscale(const Image& input, Image& output, pixel4f_t weights)
{
    output.ensureBufferSize(input.width, input.height);
    float avg;
    for (int i = 0; i < input.pixelCount; i++)
    {
        avg = brightness(input[i] * weights);
        output[i] = { avg, avg, avg, input[i].a };
    }
}

void ImagePipeline::threshold(const Image& input, Image& output, float threshold)
{
    output.ensureBufferSize(input.width, input.height);
    for (int i = 0; i < input.pixelCount; i++)
    {
        float avg = (input[i].r + input[i].g + input[i].b) / 3.0f;
        if (avg > threshold)
        {
            output[i] = { 1.0f, 1.0f, 1.0f, 1.0f };
        }
        else
        {
            output[i] = { 0.0f, 0.0f, 0.0f, 0.0f };
        }
    }
}

void ImagePipeline::thresholdColor(const Image& input, Image& output, float thresh)
{
    output.ensureBufferSize(input.width, input.height);
    for (int i = 0; i < input.pixelCount; i++)
    {
        float avg = (input[i].r + input[i].g + input[i].b) / 3.0f;
        if (avg > thresh)
        {
            output[i] = input[i];
        }
        else
        {
            output[i] = { 0.0f, 0.0f, 0.0f, 0.0f };
        }
    }
}

void ImagePipeline::colorTint(const Image& input, Image& output, pixel4f_t tint)
{
    output.ensureBufferSize(input.width, input.height);
    for (int i = 0; i < input.pixelCount; i++)
    {
        output[i] = blendOver(tint, input[i]);
    }
}

void ImagePipeline::adjustHSV(const Image& input, Image& output, pixel4f_hsv_t hsv)
{
    output.ensureBufferSize(input.width, input.height);
    for (int i = 0; i < input.pixelCount; i++)
    {
        pixel4f_hsv_t pixel = pixelRGBAtoHSVA(input[i]);
        pixel.h = pixel.h + hsv.h;
        pixel.s = pixel.s * hsv.s;
        pixel.v = pixel.v * hsv.v;
        output[i] = pixelHSVAtoRGBA(pixel);
    }
}

void ImagePipeline::gaussianBlur(const Image& input, Image& output, int kernel)
{
    temp1.ensureBufferSize(input.width, input.height);
    output.ensureBufferSize(input.width, input.height);
    if (kernel % 2 == 0)
    {
        kernel++;
    }
    int offset = int(kernel / 2);
    // Just make one corner of the kernel
    std::vector<float> convolution(offset + 1, 0);
    float stdev = float(kernel - 1) / 6.0f;
    float one_over_sqrt_2_pi_stdevsqrd = 1.0f/sqrt(2.0f * std::numbers::pi * stdev * stdev);
    for (int x = 0; x <= +offset; x++)
    {
        convolution[x] = one_over_sqrt_2_pi_stdevsqrd * exp(- (x * x) / (2.0f * stdev * stdev));
    }
    for (int x = 0; x < input.width; x++)
    {
        for (int y = 0; y < input.height; y++)
        {
            temp1(x, y) = { 0.0f, 0.0f, 0.0f, input(x, y).a };
            for (int i = -offset; i <= +offset; i++)
            {
                temp1(x, y).r += convolution[std::abs(i)] * input.clamped(x + i, y).r;
                temp1(x, y).g += convolution[std::abs(i)] * input.clamped(x + i, y).g;
                temp1(x, y).b += convolution[std::abs(i)] * input.clamped(x + i, y).b;
            }
        }
    }
    for (int x = 0; x < input.width; x++)
    {
        for (int y = 0; y < input.height; y++)
        {
            output(x, y) = { 0.0f, 0.0f, 0.0f, input(x, y).a };
            for (int j = -offset; j <= +offset; j++)
            {
                output(x, y).r += convolution[std::abs(j)] * temp1.clamped(x, y + j).r;
                output(x, y).g += convolution[std::abs(j)] * temp1.clamped(x, y + j).g;
                output(x, y).b += convolution[std::abs(j)] * temp1.clamped(x, y + j).b;
            }
        }
    }
}

void ImagePipeline::gaussianDeBlur(const Image& input, Image& output, int kernel)
{
    temp1.ensureBufferSize(input.width, input.height);
    output.ensureBufferSize(input.width, input.height);
    if (kernel % 2 == 0)
    {
        kernel++;
    }
    int offset = int(kernel / 2);
    // Just make one corner of the kernel
    std::vector<float> convolution(offset + 1, 0);
    float stdev = float(kernel - 1) / 6.0f;
    float one_over_sqrt_2_pi_stdevsqrd = 1.0f/sqrt(2.0f * std::numbers::pi * stdev * stdev);
    for (int x = 0; x <= +offset; x++)
    {
        convolution[x] = one_over_sqrt_2_pi_stdevsqrd * exp(- (x * x) / (2.0f * stdev * stdev));
    }
    SquareMatrix rowMat(input.width);
    SquareMatrix colMat(input.height);
    for (int x = 0; x < rowMat.side; x++)
    {
        for (int y = 0; y < rowMat.side; y++)
        {
            rowMat(x, y) = 0.0f;
        }
    }
    for (int x = 0; x < rowMat.side; x++)
    {
        for (int y = 0; y < rowMat.side; y++)
        {
            for (int i = -offset; i <= +offset; i++)
            if (x == y && x + i >= 0 && x + i < rowMat.side)
            {
                rowMat(y, x + i) = convolution[std::abs(i)];
            }
        }
    }
    for (int x = 0; x < colMat.side; x++)
    {
        for (int y = 0; y < colMat.side; y++)
        {
            for (int i = -offset; i <= +offset; i++)
            if (x == y && x + i >= 0 && x + i < colMat.side)
            {
                colMat(y, x + i) = convolution[std::abs(i)];
            }
        }
    }
    SquareMatrix rowMatInverse = rowMat.findInverse();
    SquareMatrix colMatInverse = colMat.findInverse();
    for (int x = 0; x < 20; x++)
    {
        for (int y = 0; y < 10; y++)
        {
            std::cout << std::fixed << std::setprecision(3) << rowMat(y, x) << " , ";
        }
        std::cout << std::endl;
    }
    for (int x = 0; x < 20; x++)
    {
        for (int y = 0; y < 10; y++)
        {
            std::cout << std::fixed << std::setprecision(3) << rowMatInverse(y, x) << " , ";
        }
        std::cout << std::endl;
    }

    // Row deblur
    for (int imgRow = 0; imgRow < input.height; imgRow++)
    {
        // Matrix row
        for (int matRow = 0; matRow < rowMatInverse.side; matRow++)
        {
            temp1(matRow, imgRow) = { 0.0f, 0.0f, 0.0f, input(matRow, imgRow).a };
            // Matrix col
            for (int matCol = 0; matCol < rowMatInverse.side; matCol++)
            {
                temp1(matRow, imgRow).r += input(matCol, imgRow).r * rowMatInverse(matRow, matCol);
                temp1(matRow, imgRow).g += input(matCol, imgRow).g * rowMatInverse(matRow, matCol);
                temp1(matRow, imgRow).b += input(matCol, imgRow).b * rowMatInverse(matRow, matCol);
            }
        }
    }
    // Col deblur
    for (int imgCol = 0; imgCol < input.width; imgCol++)
    {
        // Matrix row
        for (int matRow = 0; matRow < colMatInverse.side; matRow++)
        {
            output(imgCol, matRow) = { 0.0f, 0.0f, 0.0f, temp1(imgCol, matRow).a };
            // Matrix col
            for (int matCol = 0; matCol < colMatInverse.side; matCol++)
            {
                output(imgCol, matRow).r += temp1(imgCol, matCol).r * colMatInverse(matRow, matCol);
                output(imgCol, matRow).g += temp1(imgCol, matCol).g * colMatInverse(matRow, matCol);
                output(imgCol, matRow).b += temp1(imgCol, matCol).b * colMatInverse(matRow, matCol);
            }
        }
    }
}

void ImagePipeline::bloom(const Image& input, Image& output, float threshold, int kernel, float strength)
{
    thresholdColor(input, temp1, threshold);
    gaussianBlur(temp1, temp2, kernel);
    scaleBrightness(temp2, temp2, strength);
    add(input, temp2, output);
}

// For now, both images start at 0, 0
// Hahahahahahaa this is broken
void ImagePipeline::blendForeground(const Image& fg, const Image& bg, Image& output)
{
    int overlapWidth = std::min(fg.width, bg.width);
    int overlapHeight = std::min(fg.height, bg.height);
    int outerWidth = std::max(fg.width, bg.width);
    int outerHeight = std::max(fg.height, bg.height);
    output.ensureBufferSize(outerWidth, outerHeight);
    
    // Overlap areas
    for (int x = 0; x < overlapWidth; x++)
    {
        for (int y = 0; y < overlapHeight; y++)
        {
            output(x, y) = blendOver(fg(x, y), bg(x, y));
        }
    }
/*
    // If either bg or fg extends to the right, paint that section.
    if (fg.width > bg.width)
    {
        for (int x = bg.width; x < fg.width; x++)
        {
            for (int y = 0; y < fg.height; y++)
            {
                output[output.index(x, y)] = fg[fg.index(x, y)];
            }
        }
    }
    else
    {
        for (int x = fg.width; x < bg.width; x++)
        {
            for (int y = 0; y < bg.height; y++)
            {
                output[output.index(x, y)] = bg[bg.index(x, y)];
            }
        }
    }

    // If either bg or fg extends down, paint that section.
    if (fg.height > bg.height)
    {
        for (int x = 0; x < fg.width; x++)
        {
            for (int y = bg.height; y < fg.height; y++)
            {
                output[output.index(x, y)] = fg[fg.index(x, y)];
            }
        }
    }
    else
    {
        for (int x = 0; x < bg.width; x++)
        {
            for (int y = fg.height; y < bg.height; y++)
            {
                output[output.index(x, y)] = bg[bg.index(x, y)];
            }
        }
    }
    
    // If there's a corner that neither image covers, this paints it black
    for (int x = overlapWidth; x < outerWidth; x++)
    {
        for (int y = overlapHeight; y < outerHeight; y++)
        {
            output[output.index(x, y)] = { 0.0f, 0.0f, 0.0f, 0.0f };
        }
    }
*/
}

// TODO: Fix alpha?
void ImagePipeline::add(const Image& fg, const Image& bg, Image& output)
{
    int overlapWidth = std::min(fg.width, bg.width);
    int overlapHeight = std::min(fg.height, bg.height);
    int outerWidth = std::max(fg.width, bg.width);
    int outerHeight = std::max(fg.height, bg.height);
    output.ensureBufferSize(outerWidth, outerHeight);
    
    // Overlap areas
    for (int x = 0; x < overlapWidth; x++)
    {
        for (int y = 0; y < overlapHeight; y++)
        {
            output(x, y) = fg(x, y) + bg(x, y);
        }
    }
/*
    // If either bg or fg extends to the right, paint that section.
    if (fg.width > bg.width)
    {
        for (int x = bg.width; x < fg.width; x++)
        {
            for (int y = 0; y < fg.height; y++)
            {
                output[output.index(x, y)] = fg[fg.index(x, y)];
            }
        }
    }
    else
    {
        for (int x = fg.width; x < bg.width; x++)
        {
            for (int y = 0; y < bg.height; y++)
            {
                output[output.index(x, y)] = bg[bg.index(x, y)];
            }
        }
    }

    // If either bg or fg extends down, paint that section.
    if (fg.height > bg.height)
    {
        for (int x = 0; x < fg.width; x++)
        {
            for (int y = bg.height; y < fg.height; y++)
            {
                output[output.index(x, y)] = fg[fg.index(x, y)];
            }
        }
    }
    else
    {
        for (int x = 0; x < bg.width; x++)
        {
            for (int y = fg.height; y < bg.height; y++)
            {
                output[output.index(x, y)] = bg[bg.index(x, y)];
            }
        }
    }
    
    // If there's a corner that neither image covers, this paints it black
    for (int x = overlapWidth; x < outerWidth; x++)
    {
        for (int y = overlapHeight; y < outerHeight; y++)
        {
            output[output.index(x, y)] = { 0.0f, 0.0f, 0.0f, 0.0f };
        }
    }
*/
}

void ImagePipeline::maskify(const Image& imgIn, Image& maskOut)
{
    maskOut.ensureBufferSize(imgIn.width, imgIn.height);
    for (int i = 0; i < imgIn.pixelCount; i++)
    {
        maskOut[i] = { 0.0f, 0.0f, 0.0f, brightness(imgIn[i]) };
    }
}

void ImagePipeline::horizontalMask(Image& maskOut, float t, int feathering, int width, int height)
{
    maskOut.ensureBufferSize(width, height);
    int cutoff = clamp(int(float(width) * t), 0, width);
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            maskOut(x, y) = { 0.0f, 0.0f, 0.0f, mylerp((float(x) - cutoff) / feathering + 0.5f, 0.0f, 1.0f) };
        }
    }
}

void ImagePipeline::verticalMask(Image& maskOut, float t, int feathering, int width, int height)
{
    maskOut.ensureBufferSize(width, height);
    int cutoff = clamp(int(float(height) * t), 0, height);
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            maskOut(x, y) = { 0.0f, 0.0f, 0.0f, mylerp((float(y) - cutoff) / feathering + 0.5f, 0.0f, 1.0f) };
        }
    }
}

void ImagePipeline::circleMask(Image& maskOut, float t, int feathering, int width, int height)
{
    maskOut.ensureBufferSize(width, height);
    int centerX = width / 2;
    int centerY = height / 2;
    float finalRadius = sqrt(width * width + height * height) / 2.0f;
    int cutoff = clamp(finalRadius * t, 0.0f, finalRadius);
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            maskOut(x, y) =
            {
                0.0f,
                0.0f,
                0.0f,
                mylerp(
                    (sqrt((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY)) - cutoff) / feathering + 0.5f,
                    0.0f,
                    1.0f
                    )
            };
        }
    }
}

void ImagePipeline::composite(const Image& imgIn1, const Image& imgIn2, Image& imgOut, const Image& mask)
{
    imgOut.ensureBufferSize(imgIn1.width, imgIn1.height);
    for (int i = 0; i < imgIn1.pixelCount; i++)
    {
        imgOut[i] = imgIn1[i] * mask[i].a + imgIn2[i] * (1.0f - mask[i].a);
    }
}
