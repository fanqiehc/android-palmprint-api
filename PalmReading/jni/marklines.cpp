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
static IntImage  binImage;

void PrepareMarkLines(int wid, int hei) {
    grayImage.resize(wid, hei);
    binImage.resize(wid, hei);
}

static int findRoot(std::vector<int> &links, int x) {
    if ( links[x] != x)
        return findRoot(links, links[x]);
    return x;
}

static void BwLabel(IntImage &bwimg, std::vector<int> &labels, int top) {
    // Create black&white labeling
    int currentMaxLabel = 1;
    std::vector< int > links;
    links.push_back(0);
    
    for(int y = 0; y < bwimg.height; y++) {
        for(int x = 0; x < bwimg.width; x++) {
            if ( bwimg.data[y][x] != -1) {
                continue;
            } 

            if ( bwimg.data[y-1][x-1] > 0)
                bwimg.data[y][x] = bwimg.data[y-1][x-1];
            else if ( bwimg.data[y-1][x] > 0)
                bwimg.data[y][x] = bwimg.data[y-1][x];
            else if ( bwimg.data[y-1][x+1] > 0)
                bwimg.data[y][x] = bwimg.data[y-1][x+1];
            else if ( bwimg.data[y][x-1] > 0)
                bwimg.data[y][x] = bwimg.data[y][x-1]; 

            if ( bwimg.data[y][x] == -1 ) {
                bwimg.data[y][x] = currentMaxLabel;
                links.push_back(currentMaxLabel);
                currentMaxLabel ++;
                continue;
            }
           
            for(int yy = y - 1; yy <= y; yy++) 
            for(int xx = x - 1; xx <= x + 1; xx++){
                if ( (bwimg.data[yy][xx] > 0) && (bwimg.data[y][x] != bwimg.data[yy][xx]) ) {
                    int aroot =  findRoot(links, bwimg.data[yy][xx] );
                    int broot =  findRoot(links, bwimg.data[y][x] );
                    if ( aroot > broot) {
                        links[aroot] = broot;
                    } else {
                        links[broot] = aroot;
                    }
                }
            }
        }
    }

    std::vector<int> linkNumber;
    linkNumber.resize( currentMaxLabel + 1, 0);
    int lastv = 0;
    int lastm = 0;
    for(int y = 0; y < bwimg.height; y++) {
        for(int x = 0; x < bwimg.width; x++) {
            if( bwimg.data[y][x] > 0) {
                if ( bwimg.data[y][x] != lastv) {
                    lastv = bwimg.data[y][x];
                    lastm = findRoot( links, bwimg.data[y][x]);
                }
                bwimg.data[y][x] = lastm;
                linkNumber[ lastm ] ++;
            }
        }
    }
    linkNumber[0] = 0;  

    // select top lengest linses
    std::vector<int> linkNumberSorted = linkNumber;
    std::sort(linkNumberSorted.begin(), linkNumberSorted.end());
    
    int min_size = linkNumberSorted[ (linkNumberSorted.size() - top) < 0? 0 : linkNumberSorted.size() - top ];
    
    for(int i = 0; i < (int)linkNumber.size(); i++) {
        if ( linkNumber[i] >= min_size) {
            labels.push_back(i);
        }
    }

    for(int y = 0; y < bwimg.height; y++) {
        for(int x = 0; x < bwimg.width; x++) {
            if ( linkNumber[ bwimg.data[y][x] ] < min_size) {
                bwimg.data[y][x] = 0;
            }
        }
    } 

}

int MarkLines(unsigned char *gray_frame) {
    // copy data to local image struct
    int wid = grayImage.width;
    int hei = grayImage.height; 
   
    std::vector< std::pair<int,int> > currentMargin; 
    std::pair<int,int> pos;
    for (int y = 0; y < hei; y++) {
        for (int x = 0; x < wid; x++) {
            grayImage.data[y][x] = gray_frame[x+y*wid];
            if ( gray_frame[x+y*wid] >= 128) {
                binImage.data[y][x] = -1;
                pos.first = x;
                pos.second = y;
                currentMargin.push_back(pos);
            } else {
                binImage.data[y][x] = 0;
            }
        }   
    } 

    // diffusing image from high value
    std::vector< std::pair<int,int> > newMargin;
    while( currentMargin.size() > 0) {
        newMargin.clear();

        for(int i = 0; i < (int)currentMargin.size(); i++) {
            int x = currentMargin[i].first;
            int y = currentMargin[i].second;
            
            bool found = false;
            for(int yy = y - 1; yy <= y + 1; yy++){
                for( int xx = x - 1; xx <= x + 1; xx++){
                    if ( grayImage.data[yy][xx] >= 64 && binImage.data[yy][xx] == 0) {
                        found = true;
                    }
                }
            }
            if ( found) {
                int d = 1;
                for(int yy = y - d; yy <= y + d; yy++){
                    for( int xx = x - d; xx <= x + d; xx++){
                        pos.first = xx;
                        pos.second = yy;
                        newMargin.push_back(pos);
                        binImage.data[yy][xx] = -1;
                    }
                }
            }
        }
        currentMargin = newMargin;
    }
    // remain top five longest lines
    std::vector<int> labels;
    BwLabel(binImage, labels, 5); 

    for (int y = 0; y < hei; y++) {
        for (int x = 0; x < wid; x++) {
            if ( binImage.data[y][x] > 0) {
                gray_frame[x+y*wid] = 255;
            } 
            else {
                gray_frame[x+y*wid] = 0;
            }   
        }
    } 

    return 0;
}
