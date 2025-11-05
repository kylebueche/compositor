#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <vector>
#include <cmath>
#include "image.h"

class Viewport
{
public:
    std::vector<vec2> pixelCoords;
    Image viewport;
    
    std::vector<vec2> temp;

    Viewport(int width, int height)
    {
        viewport.ensureBufferSize(width, height);
        pixelCoords.resize(width * height);
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                int index = y * width + x;
                viewport(x, y) = { 0.0f, 0.0f, 0.0f, 1.0f };
                pixelCoords[index].x = float(x) - float(width) / 2.0f;
                pixelCoords[index].y = float(y) - float(height) / 2.0f;
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
                int leftX = int(translated.x + halfWidth);
                int rightX = leftX + 1;
                int topY = int(translated.y + halfHeight);
                int bottomY = topY + 1;
                if (leftX > 0 && rightX < image.width && topY > 0 && bottomY < image.height)
                {
                    rgba_quad_t rgbaQuad;
                    rgbaQuad.topLeft = image(leftX, topY);
                    rgbaQuad.topRight = image(rightX, topY);
                    rgbaQuad.bottomLeft = image(leftX, bottomY);
                    rgbaQuad.bottomRight = image(rightX, bottomY);

                    vec2 topLeft;
                    topLeft.x = float(leftX);
                    topLeft.y = float(topY);

                    viewport(leftX, topY) = bilinear_interpolation(translated, topLeft, rgbaQuad);
                }
            }
        }
    }
};



#endif
