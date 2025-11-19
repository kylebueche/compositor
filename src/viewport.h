#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <vector>
#include <cmath>
#include <iostream>
#include "image.h"

class Viewport
{
public:
    std::vector<vec2> pixelCoords;
    int width;
    int height;
    Image viewport;
    
    std::vector<vec2> temp;

    Viewport(int width, int height) : width(width), height(height)
    {:
        viewport.resize(width, height);
        pixelCoords.resize(width * height);
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                int index = y * width + x;
                viewport(x, y) = { 0.0f, 0.0f, 0.0f, 1.0f };
                pixelCoords[index].x = float(x);
                pixelCoords[index].y = float(y);
            }
        }
    }
    
    void clearColor(pixel4f_t color)
    {
        for (int i = 0; i < viewport.pixelCount; i++)
        {
            viewport[i] = color;
        }
    }

    void drawImage(const Image& image, vec2 scale, float rotation, vec2 translation)
    {
        float halfWidth = float(image.width) / 2.0f;
        float halfHeight = float(image.height) / 2.0f;
        temp.resize(image.width * image.height);

        // Generate the transformed image's points.
        for (int x = 0; x < image.width; x++)
        {
            for (int y = 0; y < image.height; y++)
            {
                vec2 imagePoint;
                imagePoint.x = float(x) - halfWidth;
                imagePoint.y = float(y) - halfHeight;
                
                // Scale
                vec2 scaled;
                scaled.x = imagePoint.x * scale.x;
                scaled.y = imagePoint.y * scale.y;
                
                // Rotate
                float radians = rotation * 3.14159f / 180.0f;
                float sin_theta = sin(radians);
                float cos_theta = cos(radians);
                vec2 rotated;
                rotated.x = cos_theta * scaled.x - sin_theta * scaled.y;
                rotated.y = sin_theta * scaled.x + cos_theta * scaled.y;
                
                // Translate
                vec2 translated;
                translated.x = rotated.x + translation.x;
                translated.y = rotated.y + translation.y;
                
                temp[y * image.width + x] = translated;
            }
        }

        // Top left of the top left pixel
        vec2 image_00 = temp[0];
        // Vector going in the x direction, length one pixel
        vec2 u = temp[1] - temp[0];
        // Vector going in the y direction, length one pixel
        vec2 v = temp[image.width] - temp[0];

        // h = point - image_00
        // dot(h, norm(u)) = cos(theta) * ||h|| * ||u||
        // = (adj / hyp) * hyp * 1 = adj = side length x
        // dot(h, norm(v)) = cos(theta) * ||h|| * ||v||
        // = (adj / hyp) * hyp * 1 = adj = side length y
        float len_u = length(u);
        float len_v = length(v);
        
        vec2 norm_u = normalized(u);
        vec2 norm_v = normalized(v);

        for (int x = 0; x < viewport.width; x++)
        {
            for (int y = 0; y < viewport.height; y++)
            {
                vec2 viewportPoint;
                viewportPoint.x = float(x) - halfWidth;
                viewportPoint.y = float(y) - halfHeight;
                vec2 h = viewportPoint - image_00;
                float sideLenX = dot(h, norm_u);
                float sideLenY = dot(h, norm_v);

                // Distances when pixels have side dimensions 1x1
                float distX = sideLenX / len_u;
                float distY = sideLenY / len_v;
                int topLeftX = int(distX);
                int topLeftY = int(distY);
                float tx = distX - float(topLeftX);
                float ty = distY - float(topLeftY);
                if (topLeftX >= 0 && topLeftX + 1 < image.width
                    && topLeftY >= 0 && topLeftY + 1 < image.height)
                {
                    rgba_quad_t rgbaQuad;
                    rgbaQuad.topLeft = image(topLeftX, topLeftY);
                    rgbaQuad.topRight = image(topLeftX + 1, topLeftY);
                    rgbaQuad.bottomLeft = image(topLeftX, topLeftY + 1);
                    rgbaQuad.bottomRight = image(topLeftX + 1, topLeftY + 1);
                    viewport(x, y) = bilinear_interpolation(tx, ty, rgbaQuad); // Later put bilinear interpolation here. (Or bicubic).
                }

            //    int viewportX = int(translated.x + halfWidth);
            //    int viewportY = int(translated.y + halfHeight);
            //    if (viewportX > 0 && viewportX < viewport.width
            //        && viewportY > 0 && viewportY < viewport.height
            //        && x + 1 < image.width && y + 1 < image.height)
            //    {
            //        rgba_quad_t rgbaQuad;
            //        rgbaQuad.topLeft = image(x, y);
            //        rgbaQuad.topRight = image(x + 1, y);
            //        rgbaQuad.bottomLeft = image(x, y + 1);
            //        rgbaQuad.bottomRight = image(x + 1, y + 1);

            //        vec2 topLeft;
            //        topLeft.x = float(x);
            //        topLeft.y = float(y);

            //        viewport(viewportX, viewportY) = rgbaQuad.topLeft; //bilinear_interpolation(translated, topLeft, rgbaQuad);
            }
        }
    }
};



#endif
