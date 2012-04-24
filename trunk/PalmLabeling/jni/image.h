#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <vector>

class RGBImage {
public:
    int width;
    int height;
    std::vector< std::vector<unsigned char> > data[3];
    void resize(int width, int height);
};

class ByteImage {
public:
    int width;
    int height;
    std::vector< std::vector<unsigned char> > data;
    void resize(int width, int height);
};

class FloatImage {
public:
    int width;
    int height;
    std::vector< std::vector<float> > data;
    void resize(int width, int height);
};

class IntImage {
public:
    int width;
    int height;
    std::vector< std::vector<int> > data;
    void resize(int width, int height);
};

#define SATURATE(a,min,max) ((a)<(min)?(min):((a)>(max)?(max):(a)))
inline void yuv2rgb(unsigned char inY, unsigned char inU, unsigned char inV, 
        unsigned char *R, unsigned char *G, unsigned char *B) {
    int Gi = 0, Bi = 0;
    int Y = 9535*(inY-16);
    int U = inU - 128;
    int V = inV - 128;
    int Ri = (Y + 13074*V) >> 13;
    Ri = SATURATE(Ri, 0, 255);
    Gi = (Y - 6660*V - 3203*U) >> 13;
    Gi = SATURATE(Gi, 0, 255);
    Bi = (Y + 16531*U) >> 13;
    Bi = SATURATE(Bi, 0, 255);
    *R = Ri;
    *G = Gi;
    *B = Bi;
}

inline int maxof(unsigned int a, unsigned int b) {
    if ( a >= b )
        return a;
    return b;
}

inline unsigned int maxof(unsigned int a, unsigned int b, unsigned int c) {
    int m = maxof(a,b);
    return maxof(m,c);
}

inline int minof(unsigned int a, unsigned int b) {
    if ( a <= b )
        return a;
    return b;
}

inline int minof(unsigned int a, unsigned int b, unsigned int c) {
    int m = minof(a,b);
    return minof(m,c);
}
#endif

