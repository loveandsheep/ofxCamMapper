//
//  ofxCamMapper.cpp
//  emptyExample
//
//  Created by 洋紀 加治 on 12/07/31.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#include "ofxCamMapper.h"

ofxCamMapper::ofxCamMapper(){
	
	camWin_pos = ofRectangle(640,0,CAM_WIDTH,CAM_HEIGHT);
	Buffer_src.allocate(BUFFER_WIDTH, BUFFER_HEIGHT);
	Buffer_out.allocate(BUFFER_WIDTH, BUFFER_HEIGHT);
	Buffer_invert.allocate(BUFFER_WIDTH, BUFFER_HEIGHT);
//	camera.initGrabber(CAM_WIDTH, CAM_HEIGHT, true);
	
	sampleColor.set(255,0,0);
	
	ofRegisterKeyEvents(this);
	ofRegisterMouseEvents(this);
	
	menu.menu_name = "Calib_menu";
	menu.RegisterMenu("addPoint");
	menu.RegisterMenu("PickColor");
	menu.RegisterMenu("GenPoints");

	ofAddListener(ofxCDMEvent::MenuPressed, this, &ofxCamMapper::cdmEvent);
	
	calib_waitMs = 100;
	calib_lateMs = 330;
	Genframe = 0;
	bGen_Mapping = false;
	pattern_color.set(255,0,0);
	
	src_editor.SetArea(0, 0,BUFFER_WIDTH,BUFFER_HEIGHT);
	vert_child.SetArea(0, 0, BUFFER_WIDTH, BUFFER_HEIGHT);
	mask.SetArea(0, 0, BUFFER_WIDTH, BUFFER_HEIGHT);
	
	src_editor.setChild(&vert_child);
	out_pts = &src_editor.pts;
	camWin_pos.x = 720;
	camWin_pos.width = 320;
	camWin_pos.height = 240;
	
	float flex_width = MIN(1440-BUFFER_WIDTH,320);
	float flex_height = flex_width/4.0*3.0;
	camWin_pos.set		(0, 0, CAM_WIDTH,CAM_HEIGHT);
	vert_child.SetArea	(BUFFER_WIDTH,0,flex_width,flex_height);
	src_editor.SetArea	(BUFFER_WIDTH,flex_height,flex_width,flex_height);
	mask.SetArea(0,900,3,3);
	mainView = MAINVIEW_CAMERA;
	
	inverse_affine = true;
	main_scroll.set(0, 0);
}


ofxCamMapper::~ofxCamMapper(){
	ofUnregisterMouseEvents(this);
	ofUnregisterKeyEvents(this);
}

void ofxCamMapper::update(){
	//camera.update();
	

	
	gen_Pts();
}

void ofxCamMapper::drawPanel(int x,int y){
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	ofPushMatrix();
	ofTranslate(x, y);
	
	//カメラポジションの描画
	ofSetHexColor(0xFFFFFF);
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	
	ofPushMatrix();
	ofTranslate(camWin_pos.x,camWin_pos.y);
	//camera.draw(0,0, camWin_pos.width, camWin_pos.height);
	for (int i = 0;i < cam_pts.size();i++){
		ofPoint draw_pt = ofPoint(cam_pts[i].x/CAM_WIDTH*camWin_pos.width,
								  cam_pts[i].y/CAM_HEIGHT*camWin_pos.height);
		if (i >= (*out_pts).size()){
			ofLine(draw_pt.x-3, draw_pt.y, draw_pt.x+3, draw_pt.y);
			ofLine(draw_pt.x, draw_pt.y-3, draw_pt.x, draw_pt.y+3);			
		}
	}
	ofPopMatrix();
	
	main_scroll.set(MAX(0,MIN(ofGetMouseX(),BUFFER_WIDTH-1024.0)),
					MAX(0,MIN(ofGetMouseY(),BUFFER_HEIGHT-768)));
	
	
	//メインアウトポジションの描画
	ofSetHexColor(0xFFFFFF);
	ofPushMatrix();
	ofTranslate(vert_child.drawArea.x,vert_child.drawArea.y);
	if (vert_child.enableScroll){
		Buffer_out.getTextureReference().bind();
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(vert_child.sclPt.x,vert_child.sclPt.y);
		glVertex2f(0, 0);
		glTexCoord2f(vert_child.sclPt.x+vert_child.drawArea.width,vert_child.sclPt.y);
		glVertex2f(vert_child.drawArea.width, 0);
		glTexCoord2f(vert_child.sclPt.x,vert_child.sclPt.y+vert_child.drawArea.height);
		glVertex2f(0, vert_child.drawArea.height);
		glTexCoord2f(vert_child.sclPt.x+vert_child.drawArea.width,vert_child.sclPt.y+vert_child.drawArea.height);
		glVertex2f(vert_child.drawArea.width,vert_child.drawArea.height);
		glEnd();
		Buffer_out.getTextureReference().unbind();
	}else{
		Buffer_out.draw(0, 0,vert_child.drawArea.width,vert_child.drawArea.height);
	}
	if (drawChild){
		vert_child.draw();
	}
	ofPopMatrix();

	//ソースポジションの描画
	ofSetHexColor(0xFFFFFF);
	ofPushMatrix();
	ofTranslate(src_editor.drawArea.x,src_editor.drawArea.y);
	if (src_editor.enableScroll){
		Buffer_src.getTextureReference().bind();
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(src_editor.sclPt.x,src_editor.sclPt.y);
		glVertex2f(0, 0);
		glTexCoord2f(src_editor.sclPt.x+src_editor.drawArea.width,src_editor.sclPt.y);
		glVertex2f(src_editor.drawArea.width, 0);
		glTexCoord2f(src_editor.sclPt.x,src_editor.sclPt.y+src_editor.drawArea.height);
		glVertex2f(0, src_editor.drawArea.height);
		glTexCoord2f(src_editor.sclPt.x+src_editor.drawArea.width,src_editor.sclPt.y+src_editor.drawArea.height);
		glVertex2f(src_editor.drawArea.width,src_editor.drawArea.height);
		glEnd();
		Buffer_src.getTextureReference().unbind();
	}else{
		Buffer_src.draw(0, 0,src_editor.drawArea.width,src_editor.drawArea.height);
	}
	for (int i = 0;i < src_pts.size();i++){
		ofCircle(src_pts[i].x, src_pts[i].y, 3);
	}
	ofPopMatrix();
	if (drawChild){
		src_editor.draw();
	}
	ofPopMatrix();
	
	
	//マスクの描画
	ofSetHexColor(0xFFFFFF);
	ofPushMatrix();
	ofTranslate(mask.drawArea.x, mask.drawArea.y);
	if (mask.enableScroll){
		Buffer_out.getTextureReference().bind();
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(mask.sclPt.x,mask.sclPt.y);
		glVertex2f(0, 0);
		glTexCoord2f(mask.sclPt.x+mask.drawArea.width,mask.sclPt.y);
		glVertex2f(mask.drawArea.width, 0);
		glTexCoord2f(mask.sclPt.x,mask.sclPt.y+mask.drawArea.height);
		glVertex2f(0, mask.drawArea.height);
		glTexCoord2f(mask.sclPt.x+mask.drawArea.width,mask.sclPt.y+mask.drawArea.height);
		glVertex2f(mask.drawArea.width,mask.drawArea.height);
		glEnd();
		Buffer_out.getTextureReference().unbind();
	}
	if (drawChild){
		mask.draw();		
	}
	ofPopMatrix();
	
	//キャリブレーション表示
	ofPushMatrix();
	ofTranslate(BUFFER_WIDTH, 0);
	ofSetColor(sampleColor);
	ofRect(0, 480, 40, 40);
	ofSetColor(trackingColor);
	ofRect(40, 480, 40, 40);
	string info = "";
	info += "Score: " + ofToString(color_score) + "/" + ofToString(Genframe) + "\n";
	info += "Size:" + ofToString(size) + "\n";
	info += "Elapsed:" + ofToString(ofGetElapsedTimeMillis() - calib_waiter) + "\n";
	
	ofDrawBitmapString(info, 10,540);
	
	if ((abs(trackingColor.r - sampleColor.r) < 100)&&
		(abs(trackingColor.g - sampleColor.g) < 100)&&
		(abs(trackingColor.b - sampleColor.b) < 100)){
		ofNoFill();
		ofSetHexColor(0xFFFFFF);
		ofRect(0, 480, 80, 40);
		ofFill();
	}
	ofPopMatrix();
	menu.draw();
	ofSetHexColor(0xFFFF00);
	ofLine(SelectedPt.x-5, SelectedPt.y, SelectedPt.x+5, SelectedPt.y);
	ofLine(SelectedPt.x, SelectedPt.y-5, SelectedPt.x, SelectedPt.y+5);
	
	vector<string> PresetBranch;
	PresetBranch.push_back("Msk_Preset0");
	PresetBranch.push_back("Msk_Preset1");
	PresetBranch.push_back("Msk_Preset2");
	PresetBranch.push_back("Msk_Preset3");
	PresetBranch.push_back("Msk_Preset4");
	PresetBranch.push_back("Msk_Preset5");
	PresetBranch.push_back("Msk_Preset6");
	PresetBranch.push_back("Msk_Preset7");
	PresetBranch.push_back("Msk_Preset8");
	PresetBranch.push_back("Msk_Preset9");
	mask.menu.UnRegisterMenu("Save");
	mask.menu.RegisterBranch("Save", &PresetBranch);
}

ofColor ofxCamMapper::getProjectionColor(ofPoint pts){
	unsigned char* pix;
	ofColor col_;
//	pix = camera.getPixels();
	
	col_.r = pix[(int)pts.y*3*(int)CAM_WIDTH+(int)pts.x*3];
	col_.g = pix[(int)pts.y*3*(int)CAM_WIDTH+(int)pts.x*3+1];
	col_.b = pix[(int)pts.y*3*(int)CAM_WIDTH+(int)pts.x*3+2];
	
	return col_;
}

void ofxCamMapper::gen_Pts(){
	//キャリブレーションセクション
	if (bGen_Mapping){
		if (Genframe == -1){//Initialize
			phase_x = 0;
			phase_y = 1;
			calib_next = false;
			
			currentPts = (*out_pts).size();
			
			phase = phase_x;
			head = 0;
			size = BUFFER_WIDTH;
			
			color_score = 0;
			Genframe = 0;
			calib_waiter = ofGetElapsedTimeMillis();
		}
		
		/*-------------------動的なキャリブレーションの処理------------*/
		//----------------パターンの描画
		Buffer_out.begin();
		glClearColor(0, 0, 0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ofSetRectMode(OF_RECTMODE_CORNER);
		ofSetColor(pattern_color);
		if (phase == phase_x){
			ofRect(head+size/2*calib_next, 0, size/2, BUFFER_HEIGHT);
		}
		if (phase == phase_y){
			ofRect(0,head+size/2*calib_next,BUFFER_WIDTH,size/2);
		}
		Buffer_out.end();
		
		
		//--------------カラーの比較・評価
		if ((ofGetElapsedTimeMillis() - calib_waiter) > calib_lateMs){
			trackingColor = getProjectionColor(cam_pts[currentPts]);
			if ((abs(trackingColor.r - sampleColor.r) < 60)&&
				(abs(trackingColor.g - sampleColor.g) < 60)&&
				(abs(trackingColor.b - sampleColor.b) < 60)){
				color_score++;
			}
			Genframe++;
		}
		
		//--------------比較を終了・データまとめて評価開始
		static int endChecker = 0,endCounter;
		if ((ofGetElapsedTimeMillis() - calib_waiter) > calib_waitMs+calib_lateMs){
			if ((color_score / (float)Genframe) > 0.3){
				if (!calib_next){
					size = MAX(1,size/2);
				}else{
					head = head+size/2;
					size = MAX(1,size/2);
					calib_next = false;
				}
			}else{
				if (!calib_next){
					calib_next = true;
				}else{
					head += size/3;
					calib_next = false;
				}
			}
			if (size != endChecker){
				endCounter = 0;
			}else{
				endCounter++;
			}
			endChecker = size;
			if (endCounter > 5){
				endCounter = 0;
				endChecker = 0;
				if (phase == phase_x){
					phase = phase_y;
					answer.x = head+size;
					head = 0;
					size = BUFFER_HEIGHT;
				}else{
					answer.y = head+size/2;
					currentPts++;
					
					(*out_pts).push_back(answer);
					src_editor.sync_Pts(-1);
					
					if (currentPts >= cam_pts.size()){
						bGen_Mapping = false;
					}
					phase = phase_x;
					head = 0;
					size = BUFFER_WIDTH;
					color_score = 0;
					Genframe = 0;
					calib_waiter = ofGetElapsedTimeMillis();
				}
			}
			Genframe = 0;
			color_score = 0;
			calib_waiter = ofGetElapsedTimeMillis();
		}
		
	}else{
		Buffer_out.begin();
		glClearColor(0, 0, 0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ofSetColor(pattern_color);
		if ((*out_pts).size() == 0)
		{
			ofRect(0, 0, BUFFER_WIDTH, BUFFER_HEIGHT);
			Buffer_out.end();
		}else {
			
			ofSetHexColor(0xFFFFFF);
			Buffer_src.getTextureReference().bind();
			for (int i = 0;i < src_editor.tris.size();i++){
				glBegin(GL_TRIANGLE_STRIP);
				for (int j = 0;j < 3;j++){
					glTexCoord2f(src_editor.pts[src_editor.tris[i].idx[j]].x, 
								 src_editor.pts[src_editor.tris[i].idx[j]].y);
					glVertex2f(vert_child.pts[vert_child.tris[i].idx[j]].x,
							   vert_child.pts[vert_child.tris[i].idx[j]].y);
				}
				glEnd();
			}
			Buffer_src.getTextureReference().unbind();
			Buffer_out.end();
			
			if (inverse_affine){
				for (int i = 0;i < src_editor.rects.size();i++){
					pers_rectangle pr;
					pers_rectangle_invert pri;
					
					for (int j = 0;j < 4;j++){
						pr.srcp[j]  = src_editor.pts[src_editor.rects[i].idx[j]] / ofPoint(BUFFER_WIDTH,BUFFER_HEIGHT);
						pr.pts[j]   = vert_child.pts[src_editor.rects[i].idx[j]] / ofPoint(BUFFER_WIDTH,BUFFER_HEIGHT);
						pri.srcp[j]  = src_editor.pts[src_editor.rects[i].idx[j]] / ofPoint(BUFFER_WIDTH,BUFFER_HEIGHT);
						pri.pts[j]   = src_editor.pts[src_editor.rects[i].idx[j]] / ofPoint(BUFFER_WIDTH,BUFFER_HEIGHT);
					}
					pri.pts[0] = ofPoint(0,0);
					pri.pts[1] = ofPoint(1,0);
					pri.pts[2] = ofPoint(1,1);
					pri.pts[3] = ofPoint(0,1);
					
					pr.srcp[0] = ofPoint(0,0);
					pr.srcp[1] = ofPoint(1,0);
					pr.srcp[2] = ofPoint(1,1);
					pr.srcp[3] = ofPoint(0,1);
					
					//逆行列の計算する
					Buffer_invert.begin();
					ofClear(0, 0, 0);
					Buffer_src.getTextureReference().bind();
					glPushMatrix();
					pri.setMatrix(BUFFER_WIDTH, BUFFER_HEIGHT);
					glBegin(GL_TRIANGLE_STRIP);
					
					glTexCoord2f(src_editor.pts[src_editor.rects[i].idx[0]].x,
								 src_editor.pts[src_editor.rects[i].idx[0]].y);
					glVertex2f(src_editor.pts[src_editor.rects[i].idx[0]].x,
							   src_editor.pts[src_editor.rects[i].idx[0]].y);

					glTexCoord2f(src_editor.pts[src_editor.rects[i].idx[1]].x,
								 src_editor.pts[src_editor.rects[i].idx[1]].y);
					glVertex2f(src_editor.pts[src_editor.rects[i].idx[1]].x,
							   src_editor.pts[src_editor.rects[i].idx[1]].y);

					glTexCoord2f(src_editor.pts[src_editor.rects[i].idx[3]].x,
								 src_editor.pts[src_editor.rects[i].idx[3]].y);
					glVertex2f(src_editor.pts[src_editor.rects[i].idx[3]].x,
							   src_editor.pts[src_editor.rects[i].idx[3]].y);

					glTexCoord2f(src_editor.pts[src_editor.rects[i].idx[2]].x,
								 src_editor.pts[src_editor.rects[i].idx[2]].y);
					glVertex2f(src_editor.pts[src_editor.rects[i].idx[2]].x,
							   src_editor.pts[src_editor.rects[i].idx[2]].y);

					glEnd();
					
					glPopMatrix();
					Buffer_src.getTextureReference().unbind();
					Buffer_invert.end();
					
					//インバースから引っ張ってくる
					Buffer_out.begin();
					Buffer_invert.getTextureReference().bind();
					glPushMatrix();
					pr.setMatrix(BUFFER_WIDTH, BUFFER_HEIGHT);
					glBegin(GL_TRIANGLE_STRIP);
					
					glTexCoord2f(0,0);
					glVertex2f  (0,0);
					
					glTexCoord2f(BUFFER_WIDTH,0);
					glVertex2f  (BUFFER_WIDTH,0);
					
					glTexCoord2f(0,BUFFER_HEIGHT);
					glVertex2f  (0,BUFFER_HEIGHT);

					glTexCoord2f(BUFFER_WIDTH,BUFFER_HEIGHT);
					glVertex2f  (BUFFER_WIDTH,BUFFER_HEIGHT);
					
					glEnd();
					glPopMatrix();
					Buffer_invert.getTextureReference().unbind();
					Buffer_out.end();
				}	
			}else{
				for (int i = 0;i < src_editor.rects.size();i++){
					Buffer_out.begin();
					Buffer_src.getTextureReference().bind();
					pers_rectangle pr;
					for (int j = 0;j < 4;j++){
						pr.srcp[j] = src_editor.pts[src_editor.rects[i].idx[j]] / ofPoint(BUFFER_WIDTH,BUFFER_HEIGHT);
						pr.pts[j]  = vert_child.pts[src_editor.rects[i].idx[j]] / ofPoint(BUFFER_WIDTH,BUFFER_HEIGHT);
					}
					glPushMatrix();
					pr.setMatrix(BUFFER_WIDTH, BUFFER_HEIGHT);
					glBegin(GL_QUADS);
					
					glTexCoord2f(src_editor.pts[src_editor.rects[i].idx[0]].x,
								 src_editor.pts[src_editor.rects[i].idx[0]].y);
					glVertex2f(0,0);
					
					glTexCoord2f(src_editor.pts[src_editor.rects[i].idx[1]].x,
								 src_editor.pts[src_editor.rects[i].idx[1]].y);
					glVertex2f(BUFFER_WIDTH,0);
					
					glTexCoord2f(src_editor.pts[src_editor.rects[i].idx[2]].x,
								 src_editor.pts[src_editor.rects[i].idx[2]].y);
					glVertex2f(BUFFER_WIDTH,BUFFER_HEIGHT);
					
					glTexCoord2f(src_editor.pts[src_editor.rects[i].idx[3]].x,
								 src_editor.pts[src_editor.rects[i].idx[3]].y);
					glVertex2f(0,BUFFER_HEIGHT);
					
					glEnd();
					glPopMatrix();
					Buffer_src.getTextureReference().unbind();
					Buffer_out.end();
				}
			}
			Buffer_out.begin();
//			ofSetHexColor(0x0);
            ofDisableAlphaBlending();
            glColor4f(0.0, 0.0, 0.0, 0.0);
			for (int i = 0;i < mask.rects.size();i++){
				glBegin(GL_TRIANGLE_STRIP);
				glVertex2f(mask.pts[mask.rects[i].idx[0]].x,
						   mask.pts[mask.rects[i].idx[0]].y);
				glVertex2f(mask.pts[mask.rects[i].idx[1]].x,
						   mask.pts[mask.rects[i].idx[1]].y);
				glVertex2f(mask.pts[mask.rects[i].idx[3]].x,
						   mask.pts[mask.rects[i].idx[3]].y);
				glVertex2f(mask.pts[mask.rects[i].idx[2]].x,
						   mask.pts[mask.rects[i].idx[2]].y);
				glEnd();
			}
			for (int i = 0;i < mask.tris.size();i++){
				glBegin(GL_TRIANGLE_STRIP);
				glVertex2f(mask.pts[mask.tris[i].idx[0]].x,
						   mask.pts[mask.tris[i].idx[0]].y);
				glVertex2f(mask.pts[mask.tris[i].idx[1]].x,
						   mask.pts[mask.tris[i].idx[1]].y);
				glVertex2f(mask.pts[mask.tris[i].idx[2]].x,
						   mask.pts[mask.tris[i].idx[2]].y);
				glEnd();
			}
			Buffer_out.end();
            ofEnableAlphaBlending();
		}
		ofSetHexColor(0xFFFFFF);
//		for (int i = 0;i < (*out_pts).size();i++){
//			ofNoFill();
//			ofSetHexColor(0xFF0000);
//			ofCircle((*out_pts)[i].x, (*out_pts)[i].y, 14);
//			ofSetHexColor(0xFFFFFF);
//			ofCircle((*out_pts)[i].x, (*out_pts)[i].y, 7);
//			ofLine((*out_pts)[i].x-5, (*out_pts)[i].y, (*out_pts)[i].x+5, (*out_pts)[i].y);
//			ofLine((*out_pts)[i].x, (*out_pts)[i].y-5, (*out_pts)[i].x, (*out_pts)[i].y+5);
//			ofFill();
//		}
		Buffer_out.begin();
		if (drawChild) {
			vert_child.buffer.draw(0, 0,BUFFER_WIDTH,BUFFER_HEIGHT);
            ofEnableBlendMode(OF_BLENDMODE_ALPHA);
			ofSetHexColor(0x0);
			for (int i = 0;i < mask.rects.size();i++){
				glBegin(GL_TRIANGLE_STRIP);
				glVertex2f(mask.pts[mask.rects[i].idx[0]].x,
						   mask.pts[mask.rects[i].idx[0]].y);
				glVertex2f(mask.pts[mask.rects[i].idx[1]].x,
						   mask.pts[mask.rects[i].idx[1]].y);
				glVertex2f(mask.pts[mask.rects[i].idx[3]].x,
						   mask.pts[mask.rects[i].idx[3]].y);
				glVertex2f(mask.pts[mask.rects[i].idx[2]].x,
						   mask.pts[mask.rects[i].idx[2]].y);
				glEnd();
			}
			for (int i = 0;i < mask.tris.size();i++){
				glBegin(GL_TRIANGLE_STRIP);
				glVertex2f(mask.pts[mask.tris[i].idx[0]].x,
						   mask.pts[mask.tris[i].idx[0]].y);
				glVertex2f(mask.pts[mask.tris[i].idx[1]].x,
						   mask.pts[mask.tris[i].idx[1]].y);
				glVertex2f(mask.pts[mask.tris[i].idx[2]].x,
						   mask.pts[mask.tris[i].idx[2]].y);
				glEnd();
			}
		}
		Buffer_out.end();
	}
}

void ofxCamMapper::cdmEvent(ofxCDMEvent &ev){
	ofPoint pointing;
	pointing.x = (SelectedPt.x - camWin_pos.x)/camWin_pos.width*CAM_WIDTH;
	pointing.y = (SelectedPt.y - camWin_pos.y)/camWin_pos.height*CAM_HEIGHT;
	
	if (ev.message == "Calib_menu::PickColor") {//基準色の設定
		sampleColor = getProjectionColor(pointing);
	}
	
	if (ev.message == "Calib_menu::addPoint") {//目標ポイントの追加
		cam_pts.push_back(pointing);
	}
	if (ev.message == "Calib_menu::GenPoints") {//キャリブレーション開始
		bGen_Mapping = true;
		Genframe = -1;
	}
}

void ofxCamMapper::mousePressed(ofMouseEventArgs & args){
	if (menu.phase == PHASE_CLICK) SelectedPt = ofPoint(args.x,args.y);
}
void ofxCamMapper::mouseReleased(ofMouseEventArgs & args){
	
}
void ofxCamMapper::mouseMoved(ofMouseEventArgs & args){
	if (camWin_pos.inside(args.x,args.y)){
		menu.Enable = true;
	}else{
		menu.Enable = false;
	}
}

void ofxCamMapper::mouseDragged(ofMouseEventArgs & args){
	
}

void ofxCamMapper::keyPressed(ofKeyEventArgs & key){
	if (key.key == 'c') sampleColor = trackingColor;
	
	float flex_width = MIN(1440-1024,320);
	float flex_height = flex_width/4.0*3.0;
	if (key.key == '1') {
		vert_child.enableScroll = false;
		src_editor.enableScroll = false;
		mainView = MAINVIEW_CAMERA;
		camWin_pos.set		(0, 0, CAM_WIDTH,CAM_HEIGHT);
		vert_child.SetArea	(1024,0,flex_width,flex_height);
		src_editor.SetArea	(1024,flex_height,flex_width,flex_height);
		mask.enableScroll = false;
		mask.SetArea(0,900,3,3);
	}
	if (key.key == '2') {
		vert_child.enableScroll = true;
		src_editor.enableScroll = false;
		mainView = MAINVIEW_PROJOUT;
		vert_child.SetArea	(0,0,1024,1024/BUFFER_WIDTH*BUFFER_HEIGHT);
		camWin_pos.set		(1024,  0, flex_width,flex_height);
		src_editor.SetArea	(1024,flex_height, flex_width,flex_height);
		mask.enableScroll = false;
		mask.SetArea(0,900,3,3);
	}
	if (key.key == '3') {
		vert_child.enableScroll = false;
		src_editor.enableScroll = true;
		mainView = MAINVIEW_SOURCE;
		src_editor.SetArea	(0,0,1024,1024/BUFFER_WIDTH*BUFFER_HEIGHT);
		camWin_pos.set		(1024,  0, flex_width,flex_height);
		vert_child.SetArea	(1024,flex_height, flex_width,flex_height);
		mask.enableScroll = false;
		mask.SetArea(0,900,3,3);
	}
	if (key.key == '4'){
		mask.enableScroll = true;
		vert_child.enableScroll = false;
		src_editor.enableScroll = false;
		mainView = MAINVIEW_MASK;
		mask.SetArea(0,0,1024,1024/BUFFER_WIDTH*BUFFER_HEIGHT);
		camWin_pos.set		(1024, flex_height*2, flex_width,flex_height);
		vert_child.SetArea	(1024,0,flex_width,flex_height);
		src_editor.SetArea	(1024,flex_height,flex_width,flex_height);
	}
	}
	if (key.key == '4'){
		mask.enableScroll = true;
		vert_child.enableScroll = false;
		src_editor.enableScroll = false;
		mainView = MAINVIEW_MASK;
		mask.SetArea(0,0,1024,1024/BUFFER_WIDTH*BUFFER_HEIGHT);
		camWin_pos.set		(1024, flex_height*2, flex_width,flex_height);
		vert_child.SetArea	(1024,0,flex_width,flex_height);
		src_editor.SetArea	(1024,flex_height,flex_width,flex_height);
	}
	
	if (key.key == ' ') inverse_affine ^= true;
	if (key.key == 'd') drawChild ^= true;
}
void ofxCamMapper::keyReleased(ofKeyEventArgs & key){

}
