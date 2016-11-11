#pragma once

#include "ofMain.h"
#include "ofxBox2d.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "utils.h"

class textureCircle : public ofxBox2dCircle {
  
public:
  float r;
  bool isDead;
  
  void setupTexture() {
    
    r = 15.0;
    ofSetCircleResolution(10);
  }
  
  void draw() {
    if(r < 31.0){ r++; }
    else { r = 15.0; }
    ofPushMatrix();
    ofTranslate(getPosition());
    ofRotateZ(getRotation());
    ofFill();
    ofDrawCircle(0, 0, r);
    ofPopMatrix();
  }
  
};

class RacketPoly : public ofxBox2dPolygon {
public:
  int player;
};

class resetRect : public ofxBox2dRect {
  
public:
  bool isDead;
  int width, height;
  ofImage img;
  
  void setupTexture() {
    width = 100;
    height = 100;
    img.load("images/reset.png");
    isDead = false;
  }
  
  void draw() {
    ofPushMatrix();
    ofTranslate(getPosition());
    ofRotateZ(getRotation());
    ofDrawRectangle(-50, -50, width, height);
    img.draw(-50,-50);
    ofPopMatrix();
  }
  
  void resetPosition() {
  }
};

class BodyData {
public:
  int  id;
  string name;
  bool isDead;
  int player;
  
  BodyData(int id, string name) {
    this->id = id;
    this->name = name;
  }
};


class Game {
  
public:
  
  ~Game();
  void setup(float frameRate, int width, int height);
  void contactStart(ofxBox2dContactArgs &e);
  void contactEnd(ofxBox2dContactArgs &e);
  void update(std::vector<std::vector<cv::Point>> contours, float racketThreshold, float maxDetectCount);
  void draw();
  bool isPlayer1Side(cv::Point2f verts[4]);
  bool isPlayer2Side(cv::Point2f verts[4]);
  bool createBall(ofPoint initialPos);
  bool destroyBall(b2Body *body);
  bool resetBall();
  void reset();
  void start();
  void createReset();
  void createRacket(cv::Point2f vertices[4], int count, int player);
  void close();
  
  ofxBox2d box2d;
  shared_ptr<textureCircle> circle;
  shared_ptr<resetRect> resetButton;
  vector <shared_ptr<RacketPoly>>  rackets;
  vector <shared_ptr<ofxBox2dRect>>  goals;
  ofImage p1GoalImg;
  ofImage p2GoalImg;
  ofSoundPlayer struckSound, goalSound, resetSound, gameSetSound;
  ofTrueTypeFont font;
  
  bool isPlayer1Goaled, isPlayer2Goaled, onClose,
  isPlayer1Win, isPlayer2Win;
  int player1Point, player2Point;
  int WINDOW_WIDTH, WINDOW_HEIGHT, WIN_POINT;
  int goalFrameCount = 0;
  int frameRate;
  int RACKET_COUNT = 2;
};



