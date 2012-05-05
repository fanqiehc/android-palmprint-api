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

/*
   wee apply a second degree curve to fitting the palmprint lines.
   y = a + b * x + c * x^2;
*/

typedef struct GroupParameter_ {
    float a;
    float b;
    float c;
}GroupParamter;

float finddet(float a1,float a2, float a3,float b1, float b2,float b3, float c1, float c2, float c3)
{
    return ((a1*b2*c3)-(a1*b3*c2)-(a2*b1*c3)+(a3*b1*c2)+(a2*b3*c1)-(a3*b2*c1)); /*expansion of a 3x3 determinant*/
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
        
        if ( labels.size() >= (unsigned int)top )
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

/*******************************************************
 LINES VERSION:
 y = a0 + a1 * x;
 a1 = (n*sum(x*y) - sum(x) * sum (y)) / (n*sum(x^2) - sum(x)^2)
 a0 = sum(y)/n - a1 * sum(x)/n

 PARABOLA VERSION:
 y = a + b*x + b*X^2;
 
 *******************************************************/
GroupParamter getLinesParameter(const std::vector< std::vector< std::pair<int,int> > > &targetLines,
                                         std::vector<int> &group) {
#if 1
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

    GroupParamter ret;
    ret.a = ( num * sum_xy - sum_x * sum_y)  / ( num * sum_xx - sum_x * sum_x) ;
    ret.b = sum_y/num - ret.a * sum_x  / num;
    return ret;
#else
    float x = 0.0;
    float y = 0.0;
    float x2 = 0.0;
    float x3 = 0.0;
    float x4 = 0.0;
    float xy = 0.0;
    float x2y = 0.0;  
    int num = 0;

    for(unsigned int n = 0; n < group.size(); n++) {
        for (unsigned int i = 0; i < targetLines[ group[n]].size() ; i++) {
            int x = targetLines[ group[n]][i].first;
            int y = targetLines[ group[n]][i].second;
            num ++;
            x += x;
            y += y;
            xy += x * y;
            x2 += x * x;
            y2 += y * y;             
            x3 += x * x * x;
            x2y += x * x * y;
            x4 += x * x * x * x;
        } 
    }
#endif
}

/*******************************************************
 y = a * x + b;
 distance = abs( -a * x + y - b) / sqrt(a*a + 1);
 *******************************************************/

float linearDistance(const std::vector< std::pair<int,int> > &line, float a, float b) {
    float sum = 0.0;
    for(unsigned int i = 0; i < line.size(); i++) {
        float dist = abs( line[i].second - a * line[i].first - b);
        dist = dist / sqrt( a*a  + 1);
        sum += dist;
    }

    return sum;
}

int kmean(const std::vector< std::vector< std::pair<int,int> > > &targetLines,
             std::vector< std::vector<int> > &linesLabel, std::vector<GroupParamter > &linesParameter ) {

    std::vector< std::vector<int> > newLabel;    
    for(unsigned int i = 0; i <  linesParameter.size(); i++) {          
        std::vector<int> emptyLabel;    
        newLabel.push_back(emptyLabel);
    }    

    for(unsigned int i = 0; i < targetLines.size(); i++) {
        float minDist = -1;
        int minLabel = -1;
        for (unsigned int n = 0; n < linesParameter.size(); n++) {
            float dist = linearDistance( targetLines[i], linesParameter[n].a, linesParameter[n].b);
            if ( (dist < minDist) || (minDist < 0) ){
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
            goto done;
        }
        for(unsigned int n = 0; n < newLabel.size(); n++ ) {
            if( linesLabel[i][n] != newLabel[i][n] ) {
                isSame = false;
                goto done;
            }                
        }
    }
done:    
    if ( isSame)
        return 1;

    // update linesParameter 
    linesLabel = newLabel;
    for(unsigned int i = 0; i < linesParameter.size(); i++) {
        GroupParamter lp;
        lp = getLinesParameter(targetLines, linesLabel[i]);
        linesParameter[i] = lp;
    }

    return -1;
}

int GroupLines( std::vector<int> &labels ) {
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
            if (  labelImage.data[y][x] == 0)
                continue;
            int newLabel = labelmap[ labelImage.data[y][x] ];
            std::pair<int,int> point;
            point.first = x;
            point.second = y;
            targetLines[newLabel].push_back ( point);
        }   
    } 
   
    // building first pass classify.
    std::vector< std::vector<int> > linesLabel;
    std::vector< GroupParamter > linesParameter;  
    for(int i = 0; i < K ; i++) {  
        GroupParamter lp;  
        std::vector<int> emptyLabel;    
        lp.a = 0.0;
        lp.b = 0.0; 
        lp.c = 0.0;
        linesParameter.push_back(lp);
        linesLabel.push_back(emptyLabel);
    }    
    std::vector<int> group;
    group.push_back( 0 );
    GroupParamter lp = getLinesParameter(targetLines, group);
    linesParameter[0] = lp;
    for(int i = 1; i < K; i++) {
        group[0] = i;
        lp = getLinesParameter(targetLines, group);
        linesParameter[i] = lp;
    }
  

#if 0
    for(int x = 0; x < labelImage.width; x++){ 
        int y = linesParameter[0].first * x + linesParameter[0].second;
        if ( y >=0 && y < labelImage.height) {
            labelImage.data[y][x] = 255;
        }
    }
    return 0;
#endif    

    for(int i = 0; i < 16; i++) { 
        if ( kmean(targetLines, linesLabel,  linesParameter) > 0) {
            break;
        }
    }

    // OK, we got 3 lines now.
    labels.clear();
    for(unsigned int i = 0; i < linesLabel.size(); i++) {
        for ( unsigned int n = 0; n < linesLabel[i].size(); n++) {
            for( unsigned int j = 0; j < targetLines[ linesLabel[i][n]].size(); j++) {
                int y = targetLines[ linesLabel[i][n]][j].second;
                int x = targetLines[ linesLabel[i][n]][j].first;
                labelImage.data[y][x] = i + 1;
            }
        }
        labels.push_back(i+1);
    }
    

    return 0;
}

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
    int maxNumber = 16;
    int minNumber = 6;
    //int splitValue = 3;
   
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
            if ( gray_frame[x+y*wid]  >= minValue ) {     
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
    BwLabel(labelImage, labels, maxNumber); 
   
#if 1 
    // split into small lines
    int leftx = 0;
    int lefty = 0;
    for (int y = 0; y < hei; y++) {
        for (int x = 0; x < wid; x++) {
            if ( labelImage.data[y][x] > 0) {
                labelImage.data[y][x] = -1;
                if ( y > lefty) {
                    leftx = x;
                    lefty = y;
                }
            }
        }
    }
    for( int y = lefty-5; y <= lefty; y++) {
        for ( int x = leftx-5; x <= leftx+5; x++) {
            labelImage.data[y][x] = 0;
        }
    } 
    labels.clear();
    BwLabel(labelImage, labels, maxNumber); 
#endif

    // try combing the candidated lines
    while ( labels.size() > (unsigned int)minNumber) {
        CombinLines(labels, 4);
    }
    
    GroupLines(labels);

    int ret = ClassifyLines(labels, leftOrRight);
    
    if ( ret > 0) {
        for (int y = 0; y < hei; y++) {
            for (int x = 0; x < wid; x++) {
                gray_frame[x+y*wid] = labelImage.data[y][x];
            }
        }
        return 1; 
    }

    return -1;
}
