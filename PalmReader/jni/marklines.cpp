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

/*******************************************************
 y = a0 + a1 * x;
 a1 = (n*sum(x*y) - sum(x) * sum (y)) / (n*sum(x^2) - sum(x)^2)
 a0 = sum(y)/n - a1 * sum(x)/n

 *******************************************************/
std::pair<float,float> getLinesParameter(const std::vector< std::vector< std::pair<int,int> > > &targetLines,
                                         std::vector<int> &group) {
    float sum_x = 0.0;
    float sum_y = 0.0;
    float sum_xy = 0.0;
    float sum_xx = 0.0;
    float sum_yy = 0.0;
    int num = 0;

    for(unsigned int n = 0; n < group.size(); n++) {
        for (unsigned int i = 0; i < targetLines[ group[n]].size() ; i++) {
            int x = targetLines[ group[n]][i].first;
            int y = targetLines[ group[n]][i].second;
            num ++;
            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_xx += x * x;
            sum_yy += y * y;             
        } 
    }

    std::pair<int,int> ret;
    ret.first = ( num * sum_xy - sum_x * sum_y) / ( num * sum_xx - sum_x * sum_x) ;
    ret.second = sum_y/num - ret.first * sum_x / num;

    return ret;
}

/*******************************************************
 y = a * x + b;
 distance = abs( -a * x + y - b) / sqrt(a*a + 1);
 *******************************************************/

float linearDistance(const std::vector< std::pair<int,int> > &line, float a, float b) {
    float sum = 0;
    for(unsigned int i = 0; i < line.size(); i++) {
        float dist = abs( line[i].second - a * line[i].first - b);
        dist = dist / sqrt( a*a  + 1);
        sum += dist;
    }

    return sum;
}

int kmean(const std::vector< std::vector< std::pair<int,int> > > &targetLines,
             std::vector< std::vector<int> > &linesLabel, std::vector< std::pair<float, float> > &linesParameter ) {

    std::vector< std::vector<int> > newLabel;    
    for(unsigned int i = 0; i <  linesParameter.size(); i++) {          
        std::vector<int> emptyLabel;    
        newLabel.push_back(emptyLabel);
    }    

    for(unsigned int i = 0; i < targetLines.size(); i++) {
        float minDist = -1;
        int minLabel = -1;
        for (unsigned int n = 0; n < linesParameter.size(); n++) {
            float dist = linearDistance( targetLines[i], linesParameter[n].first, linesParameter[n].first);
            if ( dist < minDist || minDist < 0) {
                minDist = dist;
                minLabel = n;
            }
        }   
        newLabel[minLabel].push_back(i);
    }

    // check is same with input classify ?
    bool isSame = true;
    for(unsigned int i = 0; i < linesParameter.size(); i++) {
        if (linesLabel[i].size() != newLabel[i].size() ){
            isSame = false;
            break;
        }
        for(unsigned int n = 0; n < newLabel.size(); n++ ) {
            if( linesLabel[i][n] != newLabel[i][n] ) {
                isSame = false;
                break;
            }                
        }
    }
    if ( isSame)
        return 1;

    // update linesParameter 
    linesLabel = newLabel;
    for(unsigned int i = 0; i < linesParameter.size(); i++) {
        std::pair<float,float> lp;
        lp = getLinesParameter(targetLines, linesLabel[i]);
        linesParameter[i] = lp;
    }

    return -1;
}

int ClassifyLines( std::vector<int> &labels ) {
    int K = 3;    

    if ( labels.size() <= (unsigned int)K)
        return -1;
    
    // change to 2d array of label result.
    std::map<int,int> labelmap;
    std::vector< std::vector< std::pair<int,int> > > targetLines;
    for(unsigned int i = 0; i < labels.size(); i++) {
        std::vector< std::pair<int,int> > emptyLine;        
        labelmap[ labels[i] ] = i;
        targetLines.push_back ( emptyLine );        
    }
    int wid = labelImage.width;
    int hei = labelImage.height;
    for (int y = 0; y < hei; y++) {
        for (int x = 0; x < wid; x++) {
            int newLabel = labelmap[ labelImage.data[y][x] ];
            std::pair<int,int> point;
            point.first = x;
            point.second = y;
            targetLines[newLabel].push_back ( point);
        }   
    } 
   
    // building pass classify.
    std::vector< std::vector<int> > linesLabel;
    std::vector< std::pair<float, float> > linesParameter;  
    for(int i = 0; i < K ; i++) {  
        std::pair<float, float> lp;  
        std::vector<int> emptyLabel;    
        lp.first = 0.0;
        lp.second = 0.0; 
        linesParameter.push_back(lp);
        linesLabel.push_back(emptyLabel);
    }    
    std::vector<int> group;
    group.push_back( 0 );
    std::pair<float,float> lp = getLinesParameter(targetLines, group);
    linesParameter[0] = lp;
    for(int i = 1; i < K; i++) {
        group[0] = i;
        lp = getLinesParameter(targetLines, group);
        linesParameter[i] = lp;
    }
    
    for(int i = 0; i < 10; i++) { 
        if ( kmean(targetLines, linesLabel,  linesParameter) > 0) {
            break;
        }

        LOGD("-------------------------------------");
        for(unsigned int n = 0; n < linesLabel.size(); n++) {
            LOGD(">>>>>>>>>>>>>>>>>>>>>>>%d", n);
            for(unsigned int j = 0; j < linesLabel[n].size(); j++)
                LOGD("%d,", linesLabel[n][j] );
        }
    }

    // OK, we got 3 lines now.


    return 0;
}

int MarkLines(unsigned char *gray_frame) {
    
    // copy data to local image struct and get left or right info 
    int wid = labelImage.width;
    int hei = labelImage.height;     
    int outlinelx = wid;
    int outlinerx = 0;
    int outlinety = hei;
    int outlinedy = 0;

    int maxValue = 160;
    int minValue = 96;
   
    std::vector< std::pair<int,int> > currentMargin; 
    std::pair<int,int> pos;
    for (int y = 0; y < hei; y++) {
        for (int x = 0; x < wid; x++) {
            if ( gray_frame[x+y*wid] >= maxValue) {
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
            if ( gray_frame[x+y*wid]  >= maxValue ) {     
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

    if ( leftUpSum <= 32 || leftDownSum <= 32 || rightUpSum <= 32 || rightDownSum <= 32)
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
                    if ( gray_frame[xx+yy*wid] >= minValue && labelImage.data[yy][xx] == 0) {
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

    // remain top longest lines
    std::vector<int> labels;
    BwLabel(labelImage, labels, 12); 
 
    int ret = ClassifyLines(labels);
    
    if ( ret == 0) {
        for (int y = 0; y < hei; y++) {
            for (int x = 0; x < wid; x++) {
                gray_frame[x+y*wid] = labelImage.data[y][x];
            }
        }
        return -1; 
    }

    return ret;
}
