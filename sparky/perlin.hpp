#pragma once
#include <math.h>
#include <stdlib.h>
namespace perlin_noise {
    // Permutation table (repeating)
    static const int perm[] = {
        151, 160, 137, 91, 90, 15,
        131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
        140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
        190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117,
        35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171,
        168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83,
        111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
        102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208,
        89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173,
        186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255,
        82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223,
        183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167,
        43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178,
        185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191,
        179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181,
        199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138,
        236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215,
        61, 156, 180
    };

    // Repeat the table for wraparound
#define PERM(x) (perm[(x) & 255])

// Fade function
    static double fade(double t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    // Linear interpolation
    static double lerp(double t, double a, double b) {
        return a + t * (b - a);
    }

    // Gradient function
    static double grad(int hash, double x, double y, double z) {
        int h = hash & 15;
        double u = h < 8 ? x : y;
        double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }

    // 3D Perlin noise function
    double perlin_noise_3d(double x, double y, double z) {
        int X = (int)floor(x) & 255;
        int Y = (int)floor(y) & 255;
        int Z = (int)floor(z) & 255;

        x -= floor(x);
        y -= floor(y);
        z -= floor(z);

        double u = fade(x);
        double v = fade(y);
        double w = fade(z);

        int A = PERM(X) + Y;
        int AA = PERM(A) + Z;
        int AB = PERM(A + 1) + Z;
        int B = PERM(X + 1) + Y;
        int BA = PERM(B) + Z;
        int BB = PERM(B + 1) + Z;

        return lerp(w, lerp(v, lerp(u, grad(PERM(AA), x, y, z),
            grad(PERM(BA), x - 1, y, z)),
            lerp(u, grad(PERM(AB), x, y - 1, z),
                grad(PERM(BB), x - 1, y - 1, z))),
            lerp(v, lerp(u, grad(PERM(AA + 1), x, y, z - 1),
                grad(PERM(BA + 1), x - 1, y, z - 1)),
                lerp(u, grad(PERM(AB + 1), x, y - 1, z - 1),
                    grad(PERM(BB + 1), x - 1, y - 1, z - 1))));
    }


    // Gradient vectors
    int grad2[8][2] = {
        {1, 1}, {-1, 1}, {1, -1}, {-1, -1},
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };


    // Dot product function
    static inline double dot_2d(int g[2], double x, double y) {
        return g[0] * x + g[1] * y;
    }

    // Hash function for permutation table
    static inline int permute_2d(int x, int y) {
        return perm[(perm[x] + y) & 255];
    }

    // Perlin noise function
    double perlin_noise_2d(double x, double y) {
        // Determine grid cell coordinates
        int X = (int)floor(x) & 255;
        int Y = (int)floor(y) & 255;

        // Relative x and y coordinates in the grid cell
        x -= floor(x);
        y -= floor(y);

        // Compute fade curves for x and y
        double u = fade(x);
        double v = fade(y);

        // Hash coordinates of the 4 cube corners
        int A = permute_2d(X, Y);
        int B = permute_2d(X + 1, Y);
        int C = permute_2d(X, Y + 1);
        int D = permute_2d(X + 1, Y + 1);

        // Compute the gradients at the cube corners
        double gradA = dot_2d(grad2[perm[A] & 7], x, y);
        double gradB = dot_2d(grad2[perm[B] & 7], x - 1, y);
        double gradC = dot_2d(grad2[perm[C] & 7], x, y - 1);
        double gradD = dot_2d(grad2[perm[D] & 7], x - 1, y - 1);

        // Interpolate the results
        double lerp1 = lerp(u, gradA, gradB);
        double lerp2 = lerp(u, gradC, gradD);
        return lerp(v, lerp1, lerp2);
    }
    // Function to generate island-like noise
    double island_noise(double x, double y) {
        double scale = 0.1; // Controls the size of features
        double noise = perlin_noise_2d(x * scale, y * scale);

        double distance = sqrt(x * x + y * y);
        double falloff = exp(-distance * 0.01); // Adjusted for a more gradual falloff

        // Ensure noise value does not fall below a threshold
        double min_threshold = 0.1;
        return fmax(noise * falloff, min_threshold);
    }
    float noise(float x, float y) {
        return perlin_noise_2d(x, y); // Simple Perlin noise function
    }

    float terrainHeight(float x, float y) {
        float baseFrequency = 0.02; // Smooth, large-scale features
        float detailFrequency = 0.1; // Finer details
        float detailAmplitude = 0.2; // Subtle bumps

        float baseNoise = noise(x * baseFrequency, y * baseFrequency);
        float detailNoise = noise(x * detailFrequency, y * detailFrequency) * detailAmplitude;
        return baseNoise + detailNoise;
    }
}
