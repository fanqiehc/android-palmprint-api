#ifndef _PALMAPI_H_
#define _PALMAPI_H_

void PrepareLabelPalm(int wid, int hei);

int LabelCentralArea(unsigned char *nv21_frame, int wid, int hei);
int LabelPalmArea(unsigned char *nv21_frame, int wid, int hei);

#endif
