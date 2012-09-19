//
//  ofxCamMapper.h
//  emptyExample
//
//  Created by 洋紀 加治 on 12/07/31.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#include "ofMain.h"
#include "ofxClickDownMenu.h"
#include "ofxMultiPointEditor.h"
#include "pers_rectangle.h"
#include "pers_rectangle_invert.h"

#define PHASE_CAMERA 0
#define PHASE_OUT 1
#define PHASE_SRC 2

//演算の関係でfloat型にしといて欲しい
#define BUFFER_WIDTH 1920.0f
#define BUFFER_HEIGHT 1080.0f
#define CAM_WIDTH 640.0f
#define CAM_HEIGHT 480.0f

#define MAINVIEW_CAMERA 0
#define MAINVIEW_PROJOUT 1
#define MAINVIEW_SOURCE 2

class ofxCamMapper{
public:
	ofxCamMapper();
	~ofxCamMapper();
	
	void update();
	void drawPanel(int x,int y);
	
	ofColor getProjectionColor(ofPoint pts);
	void gen_Pts();
	
	void mousePressed(ofMouseEventArgs & args);
	void mouseReleased(ofMouseEventArgs & args);
	void mouseMoved(ofMouseEventArgs & args);
	void mouseDragged(ofMouseEventArgs & args);
	
	void keyPressed(ofKeyEventArgs & key);
	void keyReleased(ofKeyEventArgs & key);
	void cdmEvent(ofxCDMEvent & ev);
	
	ofFbo Buffer_src;
	ofFbo Buffer_out;
	ofFbo Buffer_invert;
	ofVideoGrabber camera;
	
	deque<ofPoint> src_pts;//打ちたいソースのポイント
	deque<ofPoint> cam_pts;//カメラ上での目標ポイント
	deque<ofPoint>* out_pts;//キャリブレーションした出力ポイント
	
	ofPoint SelectedPt;
	ofColor sampleColor;
	ofColor trackingColor;
	
	ofxClickDownMenu menu;

	bool bGen_Mapping;
	int Genframe;
	int calib_waitMs;
	int calib_lateMs;
	int currentPts;
	
	ofRectangle camWin_pos;
	
	//キャリブレーション用メンバ変数
	int phase_x,phase_y;
	int phase,head,size,color_score,calib_waiter;
	bool calib_next;
	bool inverse_affine;
	ofPoint answer;
	ofColor pattern_color;
	
	ofxMultiPointEditor src_editor,vert_child;
	int mainView;
	ofPoint main_scroll;
	bool drawChild;
};