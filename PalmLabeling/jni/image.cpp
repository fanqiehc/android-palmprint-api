#include "image.h"

void RGBImage::resize(int wid, int hei) {
    width = wid;
    height = hei;

    for(int n=0; n < 3; n++) {
        data[n].resize(height);
        for(int i = 0; i < height; i++) {
            data[n][i].resize(width, 0);
        }
    }
}

void ByteImage::resize(int wid, int hei) {
    width = wid;
    height = hei;

    data.resize(height);
    for(int i = 0; i < height; i++) {
        data[i].resize(width, 0);
    }
}

void FloatImage::resize(int wid, int hei) {
    width = wid;
    height = hei;

    data.resize(height);
    for(int i = 0; i < height; i++) {
        data[i].resize(width, 0.0);
    }
}

void IntImage::resize(int wid, int hei) {
    width = wid;
    height = hei;

    data.resize(height);
    for(int i = 0; i < height; i++) {
        data[i].resize(width, 0);
    }
}

void RGB2Gray(unsigned char *rgb_frame, int wid, int hei) {
    
    unsigned char *color;
    int r,g,b;
    for(int y = 0; y < hei; y++) {
        for(int x = 0; x < wid; x++) {
            color = &rgb_frame[ (x+y*wid) * 4 ];
            r = color[1];
            g = color[2];
            b = color[3];
            color = &rgb_frame[ (x+y*wid) ];
            *color = (unsigned char) ((r*76 + g*150 + b*30) >> 8);
        }
    }

}
