//
//  TSPSDelegate.cpp
//  openTSPS
//
//  Created by rockwell on 2/14/12.
//  Copyright 2012 Rockwell Group. All rights reserved.
//

#include "TSPSDelegate.h"

//------------------------------------------------------------------------
TSPSDelegate::TSPSDelegate( int which, bool _bUseVideoFile ){
	camWidth = 640;
	camHeight = 480;
    bUseVideoFile = _bUseVideoFile;
    
    // allocate images + setup people tracker
    grayImg.allocate(camWidth, camHeight, OF_IMAGE_GRAYSCALE);
    
    bKinect         = false;
    cameraState     = CAMERA_NOT_INITED;
    
    peopleTracker.setup(camWidth, camHeight, "settings/settings"+ofToString(which)+".xml");
    peopleTracker.loadFont("fonts/times.ttf", 10);
    peopleTracker.setActiveDimensions( ofGetWidth(), ofGetHeight()-68 );
}

//------------------------------------------------------------------------
TSPSDelegate::~TSPSDelegate(){
    exit();
}

//------------------------------------------------------------------------
void TSPSDelegate::exit(){
    kinect.close();    
}

//------------------------------------------------------------------------
bool TSPSDelegate::openCamera( int which, bool _bKinect ){
    bKinect = _bKinect;
    
    bool inited = false;
    
    // are we loading from a video file?
    if ( peopleTracker.useVideoFile() || bUseVideoFile ){
        if ( bUseVideoFile && !peopleTracker.useVideoFile()){
            peopleTracker.setUseVideoFile();            
        } else if ( !bUseVideoFile && peopleTracker.useVideoFile() ){
            bUseVideoFile = peopleTracker.useVideoFile();
        }
        
        inited = initVideoFile();
    
    // let's load from a camera
    } else {
        inited = initVideoInput( which );    
    }
    
    return inited;
};

//------------------------------------------------------------------------
void TSPSDelegate::update(){
    if (peopleTracker.useKinect() && !bKinect){
        bKinect = true;
        peopleTracker.setUseKinect( initVideoInput( cameraIndex ) );
    } else if (!peopleTracker.useKinect() && bKinect){
        bKinect = false;
        initVideoInput( cameraIndex );
    } else if ( ( peopleTracker.useVideoFile() && !bUseVideoFile ) || ( peopleTracker.useVideoFile() &&videoFile != peopleTracker.getVideoFile() ) ){
        bUseVideoFile = initVideoFile();
    } else if ( !peopleTracker.useVideoFile() && bUseVideoFile ){
        bUseVideoFile = false;
        cameraState = CAMERA_NOT_INITED;
        initVideoInput();        
    }
    
    bool bNewFrame = false;
    
    if ( cameraState != CAMERA_NOT_INITED){
        if ( cameraState == CAMERA_KINECT ){
            kinect.update();
            bNewFrame = true;//kinect.isFrameNew();
        } else if ( cameraState == CAMERA_VIDEOGRABBER ){
            vidGrabber.grabFrame();
            bNewFrame = vidGrabber.isFrameNew();
        } else if ( cameraState == CAMERA_VIDEOFILE ){
            vidPlayer.idleMovie();
            bNewFrame = true; //vidPlayer.isFrameNew();
        }
    } else {
        bNewFrame = true;
    }
    
	if (bNewFrame){
        if ( cameraState == CAMERA_KINECT ){   
			grayImg.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
            peopleTracker.update(grayImg);
        } else if ( cameraState == CAMERA_VIDEOGRABBER ){
            grayImg.setFromPixels( vidGrabber.getPixelsRef() );
            grayImg.setImageType( OF_IMAGE_GRAYSCALE );
            peopleTracker.update(grayImg);
        } else if ( cameraState == CAMERA_VIDEOFILE ){
            grayImg.setFromPixels( vidGrabber.getPixelsRef() );
            grayImg.setImageType( OF_IMAGE_GRAYSCALE );
            peopleTracker.update(grayImg);
        } else if ( cameraState == CAMERA_NOT_INITED ){
            peopleTracker.update(grayImg);
        }
        
		// iterate through the people
		for(int i = 0; i < peopleTracker.totalPeople(); i++){
            ofxTSPS::Person* p = peopleTracker.personAtIndex(i);
            if (cameraState == CAMERA_KINECT){
                // distance is in mm, with the max val being 10 m
                // scale it by max to get it in a 0-1 range
                p->depth = kinect.getDistanceAt( p->highest )/10000.0;
            } else {
                p->depth = p->highest.z / 255.0f;
            }
		}
	}
}

//------------------------------------------------------------------------
void TSPSDelegate::draw(){    
	peopleTracker.draw();
}

//------------------------------------------------------------------------
void TSPSDelegate::disableEvents(){
    peopleTracker.disableGuiEvents();
};

//------------------------------------------------------------------------
void TSPSDelegate::enableEvents(){
    peopleTracker.enableGuiEvents();    
};

//------------------------------------------------------------------------
bool TSPSDelegate::initVideoFile(){    
    videoFile = peopleTracker.getVideoFile();
    bool loaded = vidPlayer.loadMovie( videoFile );
    
    if ( loaded ){
        vidPlayer.play();
        if ( camWidth != vidPlayer.width || camHeight != vidPlayer.height ){            
            camWidth    = vidPlayer.width;
            camHeight   = vidPlayer.height;
            peopleTracker.resize( camWidth, camHeight );
        }
        
        // reallocate
        grayImg.resize(camWidth, camHeight);
        
        cameraState = CAMERA_VIDEOFILE;        
    }
    
    return loaded;
}

//------------------------------------------------------------------------
bool TSPSDelegate::initVideoInput( int which ){
    bool bNewCameraIndex = false;
    
    if ( vidPlayer.isPlaying() ){
        vidPlayer.close();
    }
    
    if ( cameraIndex != which ) bNewCameraIndex = true;
    cameraIndex = which;
    
    if ( (bKinect && !cameraState == CAMERA_KINECT ) || ( bKinect && bNewCameraIndex) ){
        // not inited yet
        if (cameraState != CAMERA_KINECT){
            kinect.init();
        // just switching cameras
        } else {
            kinect.close();
        }
        bKinectConnected = (kinect.numTotalDevices() >= cameraIndex);
        if (!bKinectConnected){
            bKinect = false;
            ofLog( OF_LOG_ERROR, "NO KINECT AT THIS INDEX!");
            return false;
        }
        
        if ( cameraState == CAMERA_VIDEOGRABBER ){
            vidGrabber.close();
            cameraState = CAMERA_NOT_INITED;
        }
            
        kinect.init();
        //kinect.setVerbose(true);
        ofLog(OF_LOG_VERBOSE, "opening Kinect %d", which);
        bool bOpened = kinect.open( which );
        if (bOpened){
            cameraState = CAMERA_KINECT;
            //set this so we can access video settings through the interface
            peopleTracker.setVideoGrabber(&kinect, ofxTSPS::CAMERA_KINECT);
        }
        
        return bOpened;
    } else {      
        if ( cameraState == CAMERA_NOT_INITED || cameraState == CAMERA_KINECT || (cameraState == CAMERA_VIDEOGRABBER && bNewCameraIndex)){
            
            if ( cameraState == CAMERA_KINECT ){
                kinect.close();
                kinect.clear();
                cameraState = CAMERA_NOT_INITED;
            }
            
            if (cameraState == CAMERA_VIDEOGRABBER && bNewCameraIndex) vidGrabber.close();
            
            vidGrabber.setVerbose(false);
            vidGrabber.setDeviceID( which );
            //vidGrabber.videoSettings();
            bool bAvailable = vidGrabber.initGrabber(camWidth,camHeight);
            if (bAvailable){ 
                cameraState = CAMERA_VIDEOGRABBER;
                //set this so we can access video settings through the interface
                peopleTracker.setVideoGrabber(&vidGrabber, ofxTSPS::CAMERA_VIDEOGRABBER);
            }
            return bAvailable;
        }
        return true; // already inited, good to go
    }
    
};

//--------------------------------------------------------------
void TSPSDelegate::closeVideoInput(){
    if ( cameraState == CAMERA_KINECT ){
        kinect.close();
        kinect.clear();
        cameraState = CAMERA_NOT_INITED;
    } else if ( cameraState == CAMERA_VIDEOGRABBER ){
        vidGrabber.close();
        cameraState = CAMERA_NOT_INITED;
    }
}