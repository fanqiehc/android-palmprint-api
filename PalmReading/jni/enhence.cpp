#include <cmath>
#include <utility>
#include <algorithm>
#include <list>

#include <jni.h>
#include <android/log.h>
#include "image.h"

#define  LOG_TAG    "TEAONLY"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)  

static ByteImage  grayImage;
static IntImage intImage;
static FloatImage valueImage;

static void CreateIntegraledImage(ByteImage &gray, IntImage &inti){
    
    int sum = 0;
    for(int x = 0; x < inti.width; x++) {
        sum += gray.data[0][x];
        inti.data[0][x] = sum;
    }

    for(int y = 1; y < inti.height; y++) {
        sum = 0;
        for(int x = 0; x < inti.width; x++) {
            sum += gray.data[y][x];
            inti.data[y][x] = sum + inti.data[y-1][x] + sum;
        }
    }
}

static int BoxFilter(IntImage &inti, int bx, int by, int w, int h) {
    int x = minof(bx, inti.width);
    int y = minof(by, inti.height);
    x = maxof(x, 0);
    y = maxof(y, 0);
   
    int ex = minof(x + w, inti.width);
    int ey = minof(y + h, inti.height);

    return inti.data[ey][ex] + inti.data[y][x] - inti.data[ey][x] - inti.data[y][ex];
}

static void FastHessian(IntImage &inti, int x, int y, float* dxx, float* dxy, float *dyy) {
    int lx = BoxFilter(inti, x-4, y-2, 3, 5);
    int rx = BoxFilter(inti, x+2, y-2, 3, 5);
    int cx = BoxFilter(inti, x-1, y-2, 3, 5);
    *dxx = (lx+rx-2*cx) / 81.0;

    int uy = BoxFilter(inti, x-2, y-4, 5, 3);
    int dy = BoxFilter(inti, x-2, y+2, 5, 3);
    int cy = BoxFilter(inti, x-2, y-1, 5, 3);
    *dyy = (uy+dy-2*cy) / 81.0;

    int ul = BoxFilter(inti, x-3, y-3, 3, 3);
    int ur = BoxFilter(inti, x+1, y-3, 3, 3);
    int dl = BoxFilter(inti, x-3, y+1, 3, 3);
    int dr = BoxFilter(inti, x+1, y+1, 3, 3);
    *dxy = (ul - ur - dl + dr) / 81.0;
}

static void ComputeEigen(float dxx, float dxy, float dyy, std::vector<double> &ret) {
    //% Compute the eigenvectors of J, v1 and v2
    double tmp = sqrt((dxx - dyy)*(dxx-dyy) + 4*dxy*dxy);
    double v2x = 2*dxy; 
    double v2y = dyy - dxx + tmp;

    //% Normalize
    double mag = sqrt(v2x*v2x + v2y*v2y); 
    if ( mag != 0) {
        v2x = v2x/mag;
        v2y = v2y/mag;
    }

    //% The eigenvectors are orthogonal
    double v1x = -1 * v2y; 
    double v1y = v2x;

    //% Compute the eigenvalues
    double mu1 = 0.5*(dxx + dyy + tmp);
    double mu2 = 0.5*(dxx + dyy - tmp);

    //% Sort eigen values by absolute value abs(Lambda1)<abs(Lambda2)
    ret.resize(4, 0.0);
    ret[0] = mu2;
    ret[1] = mu1;
    
    ret[2] = v2x;
    ret[3] = v2y;

    if ( abs(mu2)>abs(mu1) ) {
        ret[0] = mu1;
        ret[1] = mu2;
        ret[2] = v1x;
        ret[3] = v1y;
    }
}

static float FrangiValue(std::vector<double> &eig) {
    const double A = 0.5;
    const double B = 15;
    
    double beta = 2 * A * A;
    double c = 2 * B * B;

    if ( eig[1] == 0 )
        eig[1] = 1 / 1e10;
    double Rb = (eig[0]/eig[1]) * (eig[0]/eig[1]);
    double S2 = eig[0]*eig[0] + eig[1]*eig[1];
    
    double ret = exp(-1*Rb/beta) * (1-exp(-1*S2/c));
    
    if ( eig[1] < 0)
        ret = 0;
    
    return ret;
}

void FrangiFilter(unsigned char *palmMap, int scale) {
    // Create integraled image
    CreateIntegraledImage(grayImage, intImage);   

    // Compute frangi filter values
    float dxx,dxy,dyy;
    std::vector<double> eig; 
    for(int y = 5; y < intImage.height-5; y++) {
        for(int x = 5; x < intImage.width-5; x++) {
            int sx = x / scale;
            int sy =  y / scale;
            if (palmMap[sx + sy*intImage.width/scale] == 0){  
                valueImage.data[y][x] = -1;
                continue;
            }
            FastHessian(intImage, x, y, &dxx, &dxy, &dyy);
            ComputeEigen(dxx, dxy, dyy, eig);
            valueImage.data[y][x] = FrangiValue(eig);
        }
    }
}

void PrepareEnhence(int wid, int hei) {
    grayImage.resize(wid, hei); 
    intImage.resize(wid, hei);
    valueImage.resize(wid, hei);
}

void EnhencePalm(unsigned char *palmMap, unsigned char *gray_frame, int scale) {
    int wid = grayImage.width;
    int hei = grayImage.height; 
    int outlinelx = wid;
    int outlinerx = 0;
    int outlinety = hei;
    int outlinedy = 0;
    for (int y = 0; y < hei; y++) {
        for (int x = 0; x < wid; x++) {
            int sx = x / scale;
            int sy =  y / scale;
            if ( palmMap[sx + sy*wid/scale] > 0) {
                if ( x > outlinerx )
                    outlinerx = x;
                if ( x < outlinelx )
                    outlinelx = x;
                if ( y > outlinedy )
                    outlinedy = y;
                if ( y < outlinety )
                    outlinety = y;
            }   
            grayImage.data[y][x] = gray_frame[x+y*wid];
            gray_frame[x+y*wid] = 0;
        }   
    } 
    int outlineHeight = outlinedy - outlinety;
    int outlineWidth = outlinerx - outlinelx;

    FrangiFilter(palmMap, scale);

    float maxvalue = 0.0;
    for (int y=(outlinety+outlineHeight/3); y <= (outlinedy-outlineHeight/3); y++) {
        for (int x=(outlinelx+outlineWidth/3); x <= (outlinerx-outlineWidth/3); x++) {
            if ( valueImage.data[y][x] > maxvalue)
                maxvalue = valueImage.data[y][x];       
        } 
    }
    maxvalue = maxvalue / 2;
    
    for (int y=(outlinety+0); y <= (outlinedy-0); y++) {
        for (int x=(outlinelx+0); x <= (outlinerx-0); x++) {
            int sx = x / scale;
            int sy =  y / scale;
            if ( palmMap[sx + sy*wid/scale] > 0) { 
                if ( valueImage.data[y][x] > maxvalue) {
                    gray_frame[x+y*wid] = 255;
                } else {
                    gray_frame[x+y*wid] = 255 * valueImage.data[y][x] / maxvalue+1;
                    //gray_frame[x+y*wid] = 1;
                }
            }
        }   
    } 

}

