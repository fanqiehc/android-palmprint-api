#include <jni.h>
#include <android/log.h>
#include <cmath>
#include "image.h"

#define  LOG_TAG    "TEAONLY"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)  

ByteImage binImage;
IntImage labelImage;

void PrepareLabelPalm(int wid, int hei) {
    binImage.resize(wid, hei);
    labelImage.resize(wid, hei);
}

int LabelCentralArea(unsigned char *nv21_frame, int wid, int hei, int scale) {
    // central area define. 
    int ltx = wid/3;
    int lty = hei/5;
    int rbx = wid/3 + wid/2;
    int rby = hei/5 + hei*3/5;
   
    int centralArea = 0;

    int uv_begin = wid * hei;
    
    unsigned char *luma, *cb, *cr;
    int lumaSum = 0;
    int cbSum = 0;
    int crSum = 0;

    for ( int y = lty; y < rby; y+=scale ) {
        for ( int x = ltx; x < rbx; x+=scale) {
            luma = &nv21_frame[x + y*wid];
            cr = &nv21_frame[ uv_begin + (y>>1)*wid + ((x>>1)<<1) ];
            cb = cr + 1;
            lumaSum += *luma;
            cbSum += *cb;
            crSum += *cr;

        }
    }

    int pointNum = (( rby - lty ) * ( rbx - ltx ) ) / ( scale * scale);
    
    float lumaMean = lumaSum / pointNum;
    float crMean = crSum / pointNum;
    float cbMean = cbSum / pointNum;
    
    float lumaDeltaSum = 0.0;
    float crDeltaSum = 0.0;
    float cbDeltaSum = 0.0;
    
    for ( int y = lty; y < rby; y+=scale ) {
        for ( int x = ltx; x < rbx; x+=scale) {
            luma = &nv21_frame[x + y*wid];
            cr = &nv21_frame[ uv_begin + (y>>1)*wid + ((x>>1)<<1) ];
            cb = cr + 1;
            lumaDeltaSum += (*luma - lumaMean) * (*luma - lumaMean);
            crDeltaSum += (*cr - crMean) * (*cr - crMean);
            cbDeltaSum += (*cb - cbMean) * (*cb - cbMean);
        }
    }
    float lumaSigma = sqrt(lumaDeltaSum / pointNum);
    float crSigma = sqrt(crDeltaSum / pointNum);
    float cbSigma = sqrt(cbDeltaSum / pointNum);
    for(int y = 0; y < hei; y+=scale ) {
        for(int x = 0; x < wid; x+=scale ) {
            labelImage.data[y/scale][x/scale] = 0;

            if ( y >= lty && y <= rby && x >= ltx && x <= rbx) {
                binImage.data[y/scale][x/scale] = 1;
                continue;
            }

            binImage.data[y/scale][x/scale] = 0;

            luma = &nv21_frame[x + y*wid];
            cr = &nv21_frame[ uv_begin + (y>>1)*wid + ((x>>1)<<1) ];
            cb = cr + 1;
 
            float diffLuma = abs(*luma - lumaMean);
            float diffCr = abs(*cr - crMean);
            float diffCb = abs(*cb - cbMean);
            if ( diffLuma < 5 * lumaSigma
                    && diffCr < 5 * crSigma
                    && diffCb < 5 * cbSigma ) {
                binImage.data[y/scale][x/scale] = 1;
                centralArea ++;
            }
        }
    } 

   return centralArea;
}

static int createDistantImage(ByteImage &map, IntImage &distImage) {

    for(int y = 1; y < distImage.height-1; y++) {
        for(int x = 1; x < distImage.width-1; x++) {
            if ( map.data[y][x] == 1)
                distImage.data[y][x] = minof( distImage.data[y][x-1] + 1, distImage.data[y-1][x] + 1);
        }
    }
    int maxv = 0; 
    for(int y = distImage.height - 2; y >= 1; y--) {
        for(int x = distImage.width - 2; x >= 1; x--) {
            if ( map.data[y][x] == 1) {
                distImage.data[y][x] = minof( distImage.data[y][x], distImage.data[y][x+1] + 1, distImage.data[y+1][x] + 1);
                if ( distImage.data[y][x] > maxv ) {
                    maxv = distImage.data[y][x];
                }
            }
        }
    }

    return maxv;
}

int LabelPalmArea(unsigned char *dst_frame) {

    int maxd = createDistantImage(binImage, labelImage);
    
    std::vector< std::pair<int,int> > currentMargin;
    std::vector< std::pair<int,int> > newMargin;
    std::pair<int,int> pos;

    unsigned char *luma;
    for(int y = 0; y < labelImage.height; y++) {
        for(int x = 0; x < labelImage.width; x++) {

            luma = &dst_frame[x + y*labelImage.width];
            *luma = 0;

            if ( labelImage.data[y][x] == maxd) {
                pos.first = x;
                pos.second = y;
                currentMargin.push_back( pos);
                binImage.data[y][x] = 1;
                *luma = 2;
            } else {
                binImage.data[y][x] = 0;
            }
        }
    }

    int mind1 = 5;
    int dist = maxd - mind1;
    if ( dist <= 0)
        dist = 0;

    for (int d = 0; d < dist; d++) {
        newMargin.clear();

        for(int i = 0; i < (int)currentMargin.size(); i++) {
            int x = currentMargin[i].first;
            int y = currentMargin[i].second;

            for ( int y1 = y - 1; y1 <= y + 1; y1 ++ ){
                for ( int x1 = x - 1; x1 <= x + 1; x1 ++ ){
                    if ( binImage.data[y1][x1] == 0 && labelImage.data[y1][x1] > mind1 ) {
                        luma = &dst_frame[x1 + y1*labelImage.width];
                        if ( (labelImage.data[y1][x1] == (mind1 + 1)) 
                                || (d == (maxd - mind1 - 1) )) {
                            *luma = 1;
                        } else {
                            *luma = 2;      
                        }
                        binImage.data[y1][x1] = 1;
                        pos.first = x1;
                        pos.second = y1;
                        newMargin.push_back(pos);
                    }
                }
            }
        }            

        if ( newMargin.size() == 0)
            break;

        currentMargin = newMargin;
    }


    return 1;
}

