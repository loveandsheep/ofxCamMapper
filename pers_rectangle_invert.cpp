//
//  pers_rectangle_invert.cpp
//  ofxMpplrExample
//
//  Created by 洋紀 加治 on 12/04/23.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#include "pers_rectangle_invert.h"

void pers_rectangle_invert::setMatrix(float width,float height){
    //lets make a matrix for openGL
	//this will be the matrix that peforms the transformation
	GLfloat m[16],im[16];
    
	//we set it to the default - 0 translation
	//and 1.0 scale for x y z and w
	for(int i = 0; i < 16; i++){
		if(i % 5 != 0) m[i] = 0.0;
		else m[i] = 1.0;
	}

	//we need our points as opencv points
	//be nice to do this without opencv?
	CvPoint2D32f cvsrc[4];
	CvPoint2D32f cvdst[4];
    
	//we set the warp coordinates
	//source coordinates as the dimensions of our window
    cvdst[0].x = 0;
    cvdst[0].y = 0;
    cvdst[1].x = width;
    cvdst[1].y = 0;
    cvdst[2].x = width;
    cvdst[2].y = height;
    cvdst[3].x = 0;
    cvdst[3].y = height;
    
	//corners are in 0.0 - 1.0 range
	//so we scale up so that they are at the window's scale
	for(int i = 0; i < 4; i++){
		cvsrc[i].x = srcp[i].x * width;
		cvsrc[i].y = srcp[i].y * height;
	}
    
	//we create a matrix that will store the results
	//from openCV - this is a 3x3 2D matrix that is
	//row ordered
	CvMat * translate = cvCreateMat(3,3,CV_32FC1);
	
	//this is the slightly easier - but supposidly less
	//accurate warping method 
	//cvWarpPerspectiveQMatrix(cvsrc, cvdst, translate); 
    
    
	//for the more accurate method we need to create
	//a couple of matrixes that just act as containers
	//to store our points  - the nice thing with this 
	//method is you can give it more than four points!
	
	CvMat* src_mat = cvCreateMat( 4, 2, CV_32FC1 );
	CvMat* dst_mat = cvCreateMat( 4, 2, CV_32FC1 );
    
	//copy our points into the matrixes
	cvSetData( src_mat, cvsrc, sizeof(CvPoint2D32f));
	cvSetData( dst_mat, cvdst, sizeof(CvPoint2D32f));
    
	//figure out the warping!
	//warning - older versions of openCV had a bug
	//in this function.
	cvFindHomography(src_mat, dst_mat, translate);
	
	//get the matrix as a list of floats
	float *matrix = translate->data.fl;
    
    
	//we need to copy these values
	//from the 3x3 2D openCV matrix which is row ordered
	//
	// ie:   [0][1][2] x
	//       [3][4][5] y
	//       [6][7][8] w
	
	//to openGL's 4x4 3D column ordered matrix
	//        x  y  z  w   
	// ie:   [0][3][ ][6]
	//       [1][4][ ][7]
	//		 [ ][ ][ ][ ]
	//       [2][5][ ][8]
	//       
    
	m[0]		= matrix[0];
	m[4]		= matrix[1];
	m[12]		= matrix[2];
	
	m[1]		= matrix[3];
	m[5]		= matrix[4];
	m[13]		= matrix[5];
	
	m[3]		= matrix[6];
	m[7]		= matrix[7];
	m[15]		= matrix[8];
    
	//finally lets multiply our matrix
	//wooooo hoooo!
	glMultMatrixf(m);    
}