#ifndef TEMPORAL_SAMPLER_H
#define TEMPORAL_SAMPLER_H

#include <vector>
#include "math.h"
#include "image.h"
#include "color.h"
#include "perlin-noise.h"
#include <iostream>


class TemporalSampler
{
    public:
    std::vector<Image> inputFrames;
    std::vector<Image> outputFrames;

    int size()
    {
        return inputFrames.size();
    }
    
    void processFrame(int frameNo, int minFrameOffset, int maxFrameOffset, Image mask)
    {
        minFrameOffset = std::max(0, minFrameOffset);
        maxFrameOffset = std::min(int(inputFrames.size() - 1), maxFrameOffset);

        for (int x = 0; x < inputFrames[frameNo].width; x++)
        {
            for (int y = 0; y < inputFrames[frameNo].height; y++)
            {
                int frameOffset = int(linear_interpolation(mask(x, y).a, minFrameOffset, maxFrameOffset));
                int index = clamp(frameNo + frameOffset, 0, inputFrames.size() - 1);
                outputFrames[frameNo](x, y) = inputFrames[index](x, y);
            }
        }
    }
    
    void loadFrames(std::string stem, int frameCount, std::string extension)
    {
        inputFrames.resize(frameCount);
        outputFrames.resize(frameCount);
        for (int i = 1; i <= frameCount; i++)
        {
            std::cout << i << std::endl;
            inputFrames[i - 1].read(fileName(stem, i, extension).c_str());
            outputFrames[i - 1].resize(inputFrames[i - 1].width, inputFrames[i - 1].height);
            std::cout << i << std::endl;
        }
    }
    
    void writeFrames(std::string stem, int frameCount, std::string extension)
    {
        for (int i = 1; i <= frameCount; i++)
        {
            outputFrames[i].write(fileName(stem, i, extension).c_str());
        }
    }
};
        
        



#endif
