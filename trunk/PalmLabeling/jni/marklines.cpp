#include <cmath>
#include <utility>
#include <algorithm>
#include <list>
#include <map>

#include <jni.h>
#include <android/log.h>
#include "image.h"

#define  LOG_TAG    "TEAONLY"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)  


static IntImage labelImage;

void PrepareMarkLines(int wid, int hei) {
    labelImage.resize(wid, hei);
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
            if ( bwimg.data[y][x] == 0) {
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

    std::map<int, int> labelNumber;
    std::map<int, int> sortedLabel;
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
                if ( labelNumber.count( lastm ) == 0)
                    labelNumber[ lastm ] = 1; 
                else 
                    labelNumber[ lastm ] = labelNumber[ lastm ] + 1;
            }
        }
    }
    
    for(std::map<int,int>::iterator i = labelNumber.begin(); i != labelNumber.end(); i++) {
        sortedLabel[ i->second ] = i->first;
    }
    
    labelNumber.clear();
    for(std::map<int,int>::reverse_iterator i = sortedLabel.rbegin(); i != sortedLabel.rend(); i++) {
        if ( labels.size() >= (unsigned int)top)
            break;

        labels.push_back( i->second );
        labelNumber[ i->second ] = i->first;
    }

    for(int y = 0; y < bwimg.height; y++) {
        for(int x = 0; x < bwimg.width; x++) {
            if ( labelNumber.count( bwimg.data[y][x]) == 0) {
                bwimg.data[y][x] = 0;
            }
        }
    } 
}

#if 0
static void ClassifyLines(std::vector<int> &labels, int leftOrRight) {
    // get the outline of palm area
    int wid = labelImage.width;
    int hei = labelImage.height; 
    int outlinelx = wid;
    int outlinerx = 0;
    int outlinety = hei;
    int outlinedy = 0;
    for (int y = 0; y < hei; y++) {
        for (int x = 0; x < wid; x++) {
            if ( labelImage.data[y][x] > 0) {
                if ( x > outlinerx )
                    outlinerx = x;
                if ( x < outlinelx )
                    outlinelx = x;
                if ( y > outlinedy )
                    outlinedy = y;
                if ( y < outlinety )
                    outlinety = y;
            }   
        }   
    } 
    
    // try to find most life likes.
    for(unsigned int n = 0; n < labels.size(); n++) {
        int life = 0;
        int header = 0;
        int markv = 0;
        for( int y = outlinety; y <= outlinedy; y++) {
            for( int x = outlinelx; x <= outlinerx; x++) {
                if ( labelImage.data[y][x] == labels[n] ) {
                    float mapx = (x - outlinelx) * 1.0 / (outlinerx - outlinelx);
                    float mapy = 1 - (y - outlinety) * 1.0 / (outlinedy - outlinety);
                    if ( mapx > mapy)
                        header++;
                    else
                        life++;
                }
            }
        }      
        
        if ( header > 9*life ) {
            markv = 1;
        } else if ( life > 9*header) {
            markv = 3;
        } else {
            markv = 2;
        }

        for( int y = outlinety; y <= outlinedy; y++) {
            for( int x = outlinelx; x <= outlinerx; x++) {
                if ( labelImage.data[y][x] == labels[n] ) {
                    labelImage.data[y][x] = markv;
                }
            }
        }  
    }
}
#else
static int ClassifyLines(std::vector<int> &labels, int leftOrRight) {
    int wid = labelImage.width;
    int hei = labelImage.height;

    if ( leftOrRight == 2) {
        for (int y = 0; y < hei>>1; y++) {
            for (int x = 0; x < wid; x++) {
                int yy = hei - 1 - y;
                int temp = labelImage.data[yy][x];
                labelImage.data[yy][x] = labelImage.data[y][x];
                labelImage.data[y][x] = temp;
            }
        }    
    }

    // find the head and life
    int headTopx = 0, headTopy = hei;
    int lifeRightx = 0, lifeRighty = 0;
    for (int y = 0; y < hei; y++) {
        for (int x = 0; x < wid; x++) {
            if ( labelImage.data[y][x] > 0 ) {
                if ( y < headTopy) {
                    headTopx = x;
                    headTopy = y;
                }
                if ( x > lifeRightx) {
                    lifeRighty = y;
                    lifeRightx = x;
                }
            } 
        }
    }
    int header = labelImage.data[headTopy][headTopx];
    int life = labelImage.data[lifeRighty][lifeRightx];
    if ( header == life)      
        return -1; 

    int heart = -1;
    for(int i = 0; i < (int)labels.size(); i++) {
        if ( labels[i] != header && labels[i] != life) {
            heart = labels[i];
            break;
        }
    }


    int lifeLeftx = 0; 
    int lifeLefty = 0;
    int lifeNumber = 0;
    int markedNumber = 0;
    for (int y = 0; y < hei; y++) {
        for (int x = 0; x < wid; x++) {
            if ( labelImage.data[y][x] == life ) {
                lifeNumber ++;
                if ( y > lifeLefty) {
                    lifeLeftx = x;
                    lifeLefty = y;
                }
            } 
        }
    }
    
    for (int y = 0; y < hei; y++) {
        for (int x = 0; x < wid; x++) {
            if ( labelImage.data[y][x] == life ) {
                if ( x > lifeLeftx) {
                    markedNumber ++;
                    labelImage.data[y][x] = -1 * life;
                }
            } 
        }
    }
   

    if ( markedNumber < 8*lifeNumber/10 ) {
        heart = life;
        life = -1 * life;
    }


    for (int y = 0; y < hei; y++) {
        for (int x = 0; x < wid; x++) {
            if ( labelImage.data[y][x] == life)
                labelImage.data[y][x] = 1;
            else if ( labelImage.data[y][x] == heart)
                labelImage.data[y][x] = 2;
            else if ( labelImage.data[y][x] == header)
                labelImage.data[y][x] = 3;
            else if ( life > 0 && labelImage.data[y][x] == -1*life)
                labelImage.data[y][x] = 1;
            else
                labelImage.data[y][x] = 0;
        }
    }

    if ( leftOrRight == 2) {
        for (int y = 0; y < hei>>1; y++) {
            for (int x = 0; x < wid; x++) {
                int yy = hei - 1 - y;
                int temp = labelImage.data[yy][x];
                labelImage.data[yy][x] = labelImage.data[y][x];
                labelImage.data[y][x] = temp;
            }
        }    
    }

    return 1;
}    

#endif


static void CombinLines(std::vector<int> &labels, int dist) {
    // get the outline of palm area
    int wid = labelImage.width;
    int hei = labelImage.height; 
    int combinedValue = labels.back();

    for (int y = dist; y < hei-dist; y+=dist) {
        for (int x = dist; x < wid-dist; x+=dist) {
            int cv = 0;   
            int v = 0;
            for(int xx = x - dist; xx < x+dist; xx++) {
                for (int yy = y - dist; yy < y+dist; yy++) {
                    if ( labelImage.data[yy][xx] > 0 && labelImage.data[yy][xx] != combinedValue ) {
                        v = labelImage.data[yy][xx];
                    } else if ( labelImage.data[yy][xx] == combinedValue ) {
                        cv = combinedValue;
                    }
                    if ( v != 0 && cv != 0) {
                        combinedValue = v;
                        goto done;
                    }
                }
            }
        }
    }

done:
    if ( combinedValue == labels.back() ) {
        for (int y = 0; y < hei; y++) {
            for(int x = 0; x < wid; x++) {
                if ( labelImage.data[y][x] == labels.back() ) {
                    labelImage.data[y][x] = 0; 
                }
            }
        }
        labels.pop_back();                      //just ignor this line
    } else {
        for (int y = 0; y < hei; y++) {
            for (int x = 0; x < wid; x++) {
                if ( labelImage.data[y][x] == labels.back() )
                    labelImage.data[y][x] = combinedValue;
            }
        }
        labels.pop_back();                      //combin this line 
    }

}

int MarkLines(unsigned char *gray_frame) {
    
    // copy data to local image struct and get left or right info 
    int wid = labelImage.width;
    int hei = labelImage.height;     
    int outlinelx = wid;
    int outlinerx = 0;
    int outlinety = hei;
    int outlinedy = 0;
   
    std::vector< std::pair<int,int> > currentMargin; 
    std::pair<int,int> pos;
    for (int y = 0; y < hei; y++) {
        for (int x = 0; x < wid; x++) {
            if ( gray_frame[x+y*wid] >= 160) {
                labelImage.data[y][x] = -1;
                pos.first = x;
                pos.second = y;
                currentMargin.push_back(pos);
            } else {
                labelImage.data[y][x] = 0;
            }

            if ( gray_frame[x+y*wid] > 0) {
                if ( x > outlinerx )
                    outlinerx = x;
                if ( x < outlinelx )
                    outlinelx = x;
                if ( y > outlinedy )
                    outlinedy = y;
                if ( y < outlinety )
                    outlinety = y;
            }   
        }   
    } 

    // get the left or right
    int leftUpSum = 0;
    int leftDownSum = 0;
    int rightUpSum = 0;
    int rightDownSum = 0;
    int leftOrRight = -1;
    for (int y = 0; y < hei; y++) {
        for (int x = 0; x < wid; x++) {            
            if ( gray_frame[x+y*wid]  > 128 ) {     
                int mapy = x;
                int mapx = hei - y;           
                if ( mapx > (outlinety + outlinedy)/2 ) {                    
                    if ( mapy > (outlinelx + outlinerx)/2 ) {
                        rightDownSum ++;
                    } else {
                        rightUpSum ++;
                    }
                } else {
                    if ( mapy > (outlinelx + outlinerx)/2 ) {
                        leftDownSum ++;
                    } else {
                        leftUpSum ++;
                    }
                }
            }
        }
    }

    if ( leftUpSum == 0 || leftDownSum == 0 || rightUpSum == 0 || rightDownSum == 0)
        return -1;

    if ( rightDownSum > leftDownSum )
        leftOrRight = 2;
    else
        leftOrRight = 1;

    // diffusing image from high value
    std::vector< std::pair<int,int> > newMargin;
    while( currentMargin.size() > 0) {
        newMargin.clear();

        for(int i = 0; i < (int)currentMargin.size(); i++) {
            int x = currentMargin[i].first;
            int y = currentMargin[i].second;
            
            for(int yy = y - 1; yy <= y + 1; yy++){
                for( int xx = x - 1; xx <= x + 1; xx++){
                    if ( gray_frame[xx+yy*wid] >= 48 && labelImage.data[yy][xx] == 0) {
                        pos.first = xx;
                        pos.second = yy;
                        newMargin.push_back(pos);
                        labelImage.data[yy][xx] = -1;
                    }
                }
            }
        }
        currentMargin = newMargin;
    }

    // remain top four longest lines
    std::vector<int> labels;
    BwLabel(labelImage, labels, 16); 
 
    // try combing the candidated lines
    while ( labels.size() > 3) {
        CombinLines(labels, 4);
    }

    for (int y = 0; y < hei; y++) {
        for (int x = 0; x < wid; x++) {
            if ( labelImage.data[y][x] > 0) {
                if ( std::find(labels.begin(), labels.end(), labelImage.data[y][x])
                        == labels.end() ) {
                    LOGD("CombinLines is Error!!!!!!!");
                }
            }
        }
    } 
 
    // classify the lines based on the position
    ClassifyLines(labels, leftOrRight);

#if 1
    for (int y = 0; y < hei; y++) {
        for (int x = 0; x < wid; x++) {
            gray_frame[x+y*wid] = labelImage.data[y][x];
        }
    } 
#endif
 
    return 0;
}
