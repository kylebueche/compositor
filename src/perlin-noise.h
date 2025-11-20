#include "math.h"

// Hash lookup table as defined by Ken Perlin.  This is a randomly
// arranged array of all numbers from 0-255 inclusive.
static int permutation[] = { 
    151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

inline float fade(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}


inline float grad(int hash, float x, float y, float z)
{
    switch(hash & 0xF)
    {
        case 0x0: return  x + y;
        case 0x1: return -x + y;
        case 0x2: return  x - y;
        case 0x3: return -x - y;
        case 0x4: return  x + z;
        case 0x5: return -x + z;
        case 0x6: return  x - z;
        case 0x7: return -x - z;
        case 0x8: return  y + z;
        case 0x9: return -y + z;
        case 0xA: return  y - z;
        case 0xB: return -y - z;
        case 0xC: return  y + x;
        case 0xD: return -y + x;
        case 0xE: return  y - x;
        case 0xF: return -y - x;
        default: return 0;
    }
}

class PerlinState
{
public:
    int p[512];
    int repeat;

    PerlinState()
    {
        for (int x = 0; x < 512; x++)
        {
            p[x] = permutation[x % 256];
        }
        repeat = 0;
    }

    inline int inc(int num)
    {
        num++;
        if (repeat > 0)
        {
            num %= repeat;
        }
        return num;
    }

    float noise(float x, float y, float z)
    {
        if (repeat > 0)
        {
            x = std::fmod(x, repeat);
            y = std::fmod(y, repeat);
            z = std::fmod(z, repeat);
        }
        
        int xi = int(x) & 255;
        int yi = int(y) & 255;
        int zi = int(z) & 255;
        
        float xf = x - int(x);
        float yf = y - int(y);
        float zf = z - int(z);

        float u = fade(xf);
        float v = fade(yf);
        float w = fade(zf);

        int aaa, aba, aab, abb, baa, bba, bab, bbb;
        aaa = p[p[p[    xi ] +     yi ] +     zi ];
        aba = p[p[p[    xi ] + inc(yi)] +     zi ];
        aab = p[p[p[    xi ] +     yi ] + inc(zi)];
        abb = p[p[p[    xi ] + inc(yi)] + inc(zi)];
        baa = p[p[p[inc(xi)] +     yi ] +     zi ];
        bba = p[p[p[inc(xi)] + inc(yi)] +     zi ];
        bab = p[p[p[inc(xi)] +     yi ] + inc(zi)];
        bbb = p[p[p[inc(xi)] + inc(yi)] + inc(zi)];

        float x1, x2, y1, y2;
        x1 = linear_interpolation(u, grad(aaa, xf, yf, zf), grad(baa, xf - 1.0f, yf, zf));
        x2 = linear_interpolation(u, grad(aba, xf, yf - 1.0f, zf), grad(bba, xf - 1.0f, yf - 1.0f, zf));
        y1 = linear_interpolation(v, x1, x2);
        x1 = linear_interpolation(u, grad(aab, xf, yf, zf - 1.0f), grad(bab, xf - 1.0f, yf, zf - 1.0f));
        x2 = linear_interpolation(u, grad(abb, xf, yf - 1.0f, zf - 1.0f), grad(bbb, xf - 1.0f, yf - 1.0f, zf - 1.0f));
        y2 = linear_interpolation(v, x1, x2);
        return (linear_interpolation(w, y1, y2) + 1.0f) / 2.0f;
    }

    float octaveNoise(float x, float y, float z, int octaves, float persistence)
    {
        float total = 0.0f;
        float frequency = 1.0f;
        float amplitude = 1.0f;
        float maxValue = 0.0f;
        for (int i = 0; i < octaves; i++)
        {
            total += noise(x * frequency, y * frequency, z * frequency) * amplitude;
            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= 2.0f;
        }
        
        return total / maxValue;
    }
};
