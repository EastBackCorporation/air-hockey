#pragma once

#include <iostream>

#include "ofMain.h"
#include "ofxDatGui.h"
#include "ofxSyphon.h"
#include "ofxXmlSettings.h"
#include "ofxModal.h"
#include "ofxCv.h"

#include <librealsense/rs.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "game.h"


using namespace std;

class ofApp : public ofBaseApp{
  
public:
  void setup();
  void update();
  void draw();
  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y );
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void mouseEntered(int x, int y);
  void mouseExited(int x, int y);
  void windowResized(int w, int h);
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);
  
  void setupGui();
  void updateGui(ofEventArgs & args);
  void drawGui(ofEventArgs & args);
  
  
  // DatGui
  void onButtonEvent(ofxDatGuiButtonEvent e);
  void onToggleEvent(ofxDatGuiToggleEvent e);
  void onSliderEvent(ofxDatGuiSliderEvent e);
  void onTextInputEvent(ofxDatGuiTextInputEvent e);
  void on2dPadEvent(ofxDatGui2dPadEvent e);
  void onDropdownEvent(ofxDatGuiDropdownEvent e);
  void onColorPickerEvent(ofxDatGuiColorPickerEvent e);
  void onMatrixEvent(ofxDatGuiMatrixEvent e);
  
  ofxDatGui* gui;
  ofxDatGuiToggle * toggleCamButton;
  ofxDatGuiToggle * toggleEasyCamButton;
  ofxDatGuiToggle * onCalibrationButton;
  ofxDatGuiToggle * enableShowCalibrationPlaneButton;
  ofxDatGuiButton * loadSettingsButton;
  ofxDatGuiButton * saveSettingsButton;
  ofxDatGuiButton * resetGameButton;
  
  ofPoint p1Pos, p2Pos, p3Pos, p4Pos;
  ofxDatGui2dPad * p1Pos2dPad;
  ofxDatGui2dPad * p2Pos2dPad;
  ofxDatGui2dPad * p3Pos2dPad;
  ofxDatGui2dPad * p4Pos2dPad;
  
  ofxDatGui* gui2;
  ofPoint calibXYPos, calibSizePos;
  ofxDatGui2dPad * calibXYPos2dPad;
  ofxDatGui2dPad * calibSizePos2dPad;
  ofxDatGuiSlider * angleXSlider;
  ofxDatGuiSlider * angleYSlider;
  ofxDatGuiSlider * angleZSlider;
  ofxDatGuiSlider * calibZSlider;
  ofxDatGuiSlider * dollySlider;
  ofxDatGuiSlider * racketTheresholdSlider;
  ofxDatGuiSlider * maxDetectCountSlider;
  
  float calibZ;
  float angleX, angleY, angleZ, dolly, racketThreshold, maxDetectCount;
  
  void onModalEvent(ofxModalEvent e);
  ofxModalAlert alert;
  
  // Realsense
  rs::context ctx;
  rs::device * dev = nullptr;
  uint stream_width, stream_height;
  bool identical, isRsDeviceReady;
  rs::stream tex_stream;
  rs::extrinsics extrin;
  rs::intrinsics depth_intrin;
  rs::intrinsics tex_intrin;
  rs::intrinsics color_intrin;
  
  // Opencv
  cv::Mat homography, view, rgb;
  cv::Mat calcHomography();
  std::vector<std::vector<cv::Point>> contours;
  
  // OF
  ofEasyCam easyCam;
  ofMesh birdEyeMesh;
  ofPlanePrimitive calibPlane;
  ofFbo fbo;
  ofPixels pix, texturePix;
  ofTexture tex;
  
  // Syphon
  ofxSyphonServer mainOutputSyphonServer;
  ofxSyphonServer individualTextureSyphonServer;
  ofxSyphonClient mClient;
  ofxXmlSettings settings;
  
  // App
  bool onCalibration, onEasyCam, enableShowCalibrationPlane;
  int WINDOW_WIDTH, WINDOW_HEIGHT;
  int frameRate;
  Game game;
};




