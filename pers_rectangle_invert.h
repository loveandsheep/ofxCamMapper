//
//  pers_rectangle.h
//  ofxMpplrExample
//
//  Created by 洋紀 加治 on 12/04/23.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//
#include "ofMain.h"
#include "ofxOpenCv.h"

class pers_rectangle_invert{
public:
    ofRectangle src;
    ofPoint srcp[4];
    ofPoint pts[4];
    void setMatrix(int width,int height);
};