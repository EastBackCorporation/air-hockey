#include "ofApp.h"
#include "utils.h"
using namespace cv;
using namespace ofxCv;
using namespace std;
using namespace rs;

//--------------------------------------------------------------
void ofApp::setup(){
  try {

    // Realsense
    rs::log_to_console(rs::log_severity::warn);
    if (ctx.get_device_count() > 0){
      frameRate = 30;
      int rsFrameRate = 30;
      dev = ctx.get_device(0);
      dev->enable_stream(rs::stream::depth, 640, 480, rs::format::z16, rsFrameRate);
      dev->enable_stream(rs::stream::color, 640, 480, rs::format::rgb8, rsFrameRate);
      dev->enable_stream(rs::stream::infrared, 640, 480, rs::format::y8, rsFrameRate);
      dev->start();

      tex_stream = rs::stream::color; //depth; //rs::stream::color, rs::stream::infrared
      extrin = dev->get_extrinsics(rs::stream::depth, tex_stream);
      depth_intrin = dev->get_stream_intrinsics(rs::stream::depth);
      color_intrin = dev->get_stream_intrinsics(rs::stream::color);
      tex_intrin = dev->get_stream_intrinsics(tex_stream);
      identical = depth_intrin == tex_intrin && extrin.is_identity();
      dev->set_option(rs::option::r200_lr_auto_exposure_enabled, 1);
      isRsDeviceReady = true;

      cout << "--------------------------------------------";
      printf("\nUsing device 0, an %s\n", dev->get_name());
      printf("    Serial number: %s\n", dev->get_serial());
      printf("    Firmware version: %s\n", dev->get_firmware_version());
      cout << "--------------------------------------------" << endl;

      stream_width = dev->get_stream_width(rs::stream::depth);
      stream_height = dev->get_stream_height(rs::stream::depth);

      // OF settings.
      cout << "--------------------------------------------" << endl;
      cout << "OF version: " << ofGetVersionInfo();
      cout << "--------------------------------------------" << endl;
      ofDisableAntiAliasing();
      ofBackground(COLOR_BLACK);
      ofSetColor(COLOR_BLUE);
      ofSetLogLevel(OF_LOG_NOTICE);
      ofSetVerticalSync(true);
      ofSetFrameRate(frameRate);
      WINDOW_WIDTH = ofGetWidth();
      WINDOW_HEIGHT = ofGetHeight();
      fbo.allocate(WINDOW_WIDTH, WINDOW_HEIGHT);
      birdEyeMesh.setMode(OF_PRIMITIVE_POINTS);


      calibPlane.set(2000, 2000);
      calibPlane.setPosition(0, 0, 0);
      calibPlane.setResolution(1, 1);
      tex.allocate(tex_intrin.width, tex_intrin.height, GL_RGB);

      // OpenCV settings.
      cout << "--------------------------------------------" << endl;
      cout << "OpenCV version : " << CV_VERSION << endl;
      cout << "--------------------------------------------" << endl;

      // Game settings
      game.setup(frameRate, WINDOW_WIDTH, WINDOW_HEIGHT);

      // Syphon settings.
      mainOutputSyphonServer.setName("Screen Outputh");
      mClient.setup();
      mClient.setApplicationName("Simple Serverh");
      mClient.setServerName("");
    } else {
      isRsDeviceReady = false;
    }
  }
  catch (const rs::error & e)
  {
    std::stringstream ss;
    ss << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    std::cerr << ss.str();
    for(int j=0; j<4; ++j)
    {
      auto s = (rs::stream)j;
      if(dev->is_stream_enabled(s)) dev->disable_stream(s);
    }
    ofExit();
  }
  catch (const std::exception & e)
  {
    std::cerr << e.what() << std::endl;
    for(int j=0; j<4; ++j)
    {
      auto s = (rs::stream)j;
      if(dev->is_stream_enabled(s)) dev->disable_stream(s);
    }
    ofExit();
  }
}

void ofApp::setupGui()
{
  // ofxdatGUI settings.
  gui = new ofxDatGui( ofxDatGuiAnchor::TOP_LEFT );
  gui->addFRM();
  gui->addBreak();
  toggleCamButton = gui->addToggle("Realsense");
  toggleCamButton->onToggleEvent(this, &ofApp::onToggleEvent);

  onEasyCam = false;
  toggleEasyCamButton = gui->addToggle("onEasyCam");
  toggleEasyCamButton->onToggleEvent(this, &ofApp::onToggleEvent);

  toggleCamButton->setChecked(true);
  onCalibration = true;
  onCalibrationButton = gui->addToggle("onCalibration");
  onCalibrationButton->onToggleEvent(this, &ofApp::onToggleEvent);
  onCalibrationButton->setChecked(true);

  enableShowCalibrationPlane = false;
  enableShowCalibrationPlaneButton = gui->addToggle("enableShowCalibrationPlane");
  enableShowCalibrationPlaneButton->onToggleEvent(this, &ofApp::onToggleEvent);
  enableShowCalibrationPlaneButton->setChecked(false);

  loadSettingsButton = gui->addButton("loadSettings");
  loadSettingsButton->onButtonEvent(this, &ofApp::onButtonEvent);

  p1Pos.set(100, 100);
  p2Pos.set(100, 300);
  p3Pos.set(300, 300);
  p4Pos.set(300, 100);
  p1Pos2dPad = gui->add2dPad("P1 position");
  p1Pos2dPad->on2dPadEvent(this, &ofApp::on2dPadEvent);
  p1Pos2dPad->setPoint(p1Pos);
  p2Pos2dPad = gui->add2dPad("P2 position");
  p2Pos2dPad->on2dPadEvent(this, &ofApp::on2dPadEvent);
  p2Pos2dPad->setPoint(p2Pos);
  p3Pos2dPad = gui->add2dPad("P3 position");
  p3Pos2dPad->on2dPadEvent(this, &ofApp::on2dPadEvent);
  p3Pos2dPad->setPoint(p3Pos);
  p4Pos2dPad = gui->add2dPad("P4 position");
  p4Pos2dPad->on2dPadEvent(this, &ofApp::on2dPadEvent);
  p4Pos2dPad->setPoint(p4Pos);


  saveSettingsButton = gui->addButton("saveSettings");
  saveSettingsButton->onButtonEvent(this, &ofApp::onButtonEvent);
  resetGameButton = gui->addButton("resetGame");
  resetGameButton->onButtonEvent(this, &ofApp::onButtonEvent);

  gui->addFooter();


  gui2 = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );

  calibXYPos.set(1300, 1400);
  ofRectangle rect = ofRectangle(-6000, -6000, 12000, 12000);
  calibXYPos2dPad = gui2->add2dPad("calibXY position");
  calibXYPos2dPad->on2dPadEvent(this, &ofApp::on2dPadEvent);
  calibXYPos2dPad->setBounds(rect);
  calibXYPos2dPad->setPoint(calibXYPos);

  calibSizePos.set(1300, 1400);
  calibSizePos2dPad = gui2->add2dPad("calibSize position");
  calibSizePos2dPad->on2dPadEvent(this, &ofApp::on2dPadEvent);
  calibSizePos2dPad->setBounds(rect);
  calibSizePos2dPad->setPoint(calibSizePos);

  calibZ = -1500.0;
  calibZSlider = gui2->addSlider("calibZ", -5000.0, 5000.0, calibZ);
  calibZSlider->onSliderEvent(this, &ofApp::onSliderEvent);

  angleX = 0;
  angleXSlider = gui2->addSlider("angleX", -180.0, 180.0, angleX);
  angleXSlider->onSliderEvent(this, &ofApp::onSliderEvent);

  angleY = 0;
  angleYSlider = gui2->addSlider("angleY", -180.0, 180.0, angleY);
  angleYSlider->onSliderEvent(this, &ofApp::onSliderEvent);

  angleZ = 0;
  angleZSlider = gui2->addSlider("angleZ", -180.0, 180.0, angleZ);
  angleZSlider->onSliderEvent(this, &ofApp::onSliderEvent);


  dolly = 0;
  dollySlider = gui2->addSlider("dolly", -5000.0, 5000.0, dolly);
  dollySlider->onSliderEvent(this, &ofApp::onSliderEvent);

  racketThreshold = 800;
  racketTheresholdSlider = gui2->addSlider("racketThreshold", 0.0, 1500.0, racketThreshold);
  racketTheresholdSlider->onSliderEvent(this, &ofApp::onSliderEvent);


  maxDetectCount = 50;
  maxDetectCountSlider = gui2->addSlider("maxDetectCount", 0.0, 500.0, maxDetectCount);
  maxDetectCountSlider->onSliderEvent(this, &ofApp::onSliderEvent);

  gui2->addFooter();

  alert.setWidth(ofGetWindowWidth() * 2 / 3);
  alert.addListener(this, &ofApp::onModalEvent);

}


//--------------------------------------------------------------
void ofApp::update(){
  if( isRsDeviceReady && dev->is_streaming() ){
    dev->wait_for_frames();

    auto points = reinterpret_cast<const rs::float3 *>(dev->get_frame_data(rs::stream::points));

    birdEyeMesh.clear();

    if(onCalibration) {
      //TODO: 色とポイントクラウドがずれているのを修正する。
      tex.loadData((uchar *)dev->get_frame_data(rs::stream::color), tex_intrin.width, tex_intrin.height,  GL_RGB);
      tex.readToPixels(texturePix);
      for(int y=0; y<depth_intrin.height; ++y)
      {
        for(int x=0; x<depth_intrin.width; ++x)
        {

          rs::float2 texcoord = tex_intrin.project_to_texcoord(extrin.transform(*points));
          ofColor color = texturePix.getColor( x, y );
          birdEyeMesh.addColor(color);
          birdEyeMesh.addTexCoord(ofVec2f(texcoord.x, texcoord.y));

          // TODO
          birdEyeMesh.addVertex(ofVec3f(points->x * 1500, points->y * -1500, points->z * -1500));

          ++points;
        }
      }
    } else {
      for(int y=0; y<depth_intrin.height; ++y)
      {
        for(int x=0; x<depth_intrin.width; ++x)
        {
          // TODO
          birdEyeMesh.addVertex(ofVec3f(points->x * 1500, points->y * -1500, points->z * -1500));
          ++points;
        }
      }
      game.update(contours, racketThreshold, maxDetectCount);
    }
  }
}

void ofApp::updateGui(ofEventArgs & args){}

//--------------------------------------------------------------
void ofApp::draw(){
  if(isRsDeviceReady){

    fbo.begin();

    if(onEasyCam) {
      easyCam.begin();
    }
    ofClear(COLOR_WHITE);

    ofPushMatrix();

    glPointSize(2.0);
    glEnable(GL_DEPTH_TEST);

    birdEyeMesh.draw();

    if(enableShowCalibrationPlane){
      ofSetColor(COLOR_BLACK);
      calibPlane.set(calibSizePos.x, calibSizePos.y);
      calibPlane.setPosition(calibXYPos.x, calibXYPos.y, calibZ);
      calibPlane.setOrientation(ofVec3f(angleX, angleY, angleZ));
      calibPlane.setResolution(2, 2);
      calibPlane.draw();
      ofSetColor(COLOR_WHITE);
    }


    glDisable(GL_DEPTH_TEST);

    ofPopMatrix();

    if(onEasyCam) easyCam.end();
    fbo.end();

    fbo.readToPixels(pix);

    if(onCalibration){
      fbo.draw(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
      ofSetColor(COLOR_RED);
      ofDrawLine(p1Pos, p2Pos);
      ofDrawLine(p2Pos, p3Pos);
      ofDrawLine(p3Pos, p4Pos);
      ofDrawLine(p4Pos, p1Pos);
      ofSetColor(COLOR_WHITE);
    } else {
      fbo.begin();
      game.draw();
      fbo.end();
      fbo.draw(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    }


    mClient.draw(0, 0);
    mainOutputSyphonServer.publishScreen();
  }
}


void ofApp::drawGui(ofEventArgs & args)
{
  if( isRsDeviceReady ){
    if(onCalibration && pix.size() > 0){
      cv::Mat m = toCv(pix);
      homography = calcHomography();

      cv::Mat src, gray;
      cv::warpPerspective(m, src, homography, m.size());

      cv::cvtColor(src, gray, CV_RGB2GRAY);
      view = src.clone();

      cv::bitwise_not(gray, gray);
      gray = 255 - gray;
      //cv::findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
      cv::findContours(gray, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
      cv::drawContours(view, contours, -1, cv::Scalar(0, 0, 255), 3); // BGR
      int guiWindowHeight = ofGetHeight();
      //cv::imshow("show", view);
      drawMat(view, 300.0f, guiWindowHeight/2, WINDOW_WIDTH/2, WINDOW_HEIGHT/2);

    } else if(pix.size() > 0){
      cv::Mat m = toCv(pix);

      cv::Mat src, gray;
      cv::warpPerspective(m, src, homography, m.size());

      cv::cvtColor(src, gray, CV_RGB2GRAY);
      view = src.clone();

      cv::bitwise_not(gray, gray);
      gray = 255 - gray;
      //cv::findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
      cv::findContours(gray, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    }

  } else {
    alert.setWidth(ofGetWindowWidth() * 2 / 3);
    alert.alert("Device Not Found");
  }
}

void ofApp::onButtonEvent(ofxDatGuiButtonEvent e){
  if(e.target->getLabel() == "loadSettings"){
    settings.loadFile("settings.xml");

    // easyCam
    easyCam.setPosition(settings.getValue("settings:camX", 0.0),
                        settings.getValue("settings:camY", 0.0),
                        settings.getValue("settings:camZ", 0.0));
    easyCam.setDrag( settings.getValue("settings:camDrag", 0.0) );
    ofQuaternion q = ofQuaternion( settings.getValue("settings:camQX", 0.0),
                                  settings.getValue("settings:camQY", 0.0),
                                  settings.getValue("settings:camQZ", 0.0),
                                  settings.getValue("settings:camQW", 0.0) );
    easyCam.setOrientation(q);
    // CV
    p1Pos.x = settings.getValue("settings:p1PosX", 0.0);
    p1Pos.y = settings.getValue("settings:p1PosY", 0.0);
    p2Pos.x = settings.getValue("settings:p2PosX", 0.0);
    p2Pos.y = settings.getValue("settings:p2PosY", 0.0);
    p3Pos.x = settings.getValue("settings:p3PosX", 0.0);
    p3Pos.y = settings.getValue("settings:p3PosY", 0.0);
    p4Pos.x = settings.getValue("settings:p4PosX", 0.0);
    p4Pos.y = settings.getValue("settings:p4PosY", 0.0);

    // calib
    calibXYPos.x = settings.getValue("settings:calibXYPosX", 0.0);
    calibXYPos.y = settings.getValue("settings:calibXYPosY", 0.0);
    calibSizePos.x = settings.getValue("settings:calibSizePosX", 0.0);
    calibSizePos.y = settings.getValue("settings:calibSizePosY", 0.0);
    calibZ = settings.getValue("settings:calibZ", 0.0);
    angleX = settings.getValue("settings:angleX", 0.0);
    angleY = settings.getValue("settings:angleY", 0.0);
    angleZ = settings.getValue("settings:angleZ", 0.0);
    dolly = settings.getValue("settings:dolly", 0.0);
    frameRate = settings.getValue("settings:frameRate", 30);

    racketThreshold = settings.getValue("settings:racketThreshold", 0.0);
    maxDetectCount = settings.getValue("settings:maxDetectCount", 0.0);

    calibZSlider->setValue(calibZ);
    angleXSlider->setValue(angleX);
    angleYSlider->setValue(angleY);
    angleZSlider->setValue(angleZ);
    racketTheresholdSlider->setValue(racketThreshold);
    maxDetectCountSlider->setValue(maxDetectCount);

    cout << "loaded" << endl;

  } else if(e.target->getLabel() == "saveSettings"){
    ofVec3f camPos = easyCam.getPosition();
    settings.setValue("settings:camX", camPos.x);
    settings.setValue("settings:camY", camPos.y);
    settings.setValue("settings:camZ", camPos.z);

    settings.setValue("settings:camDrag",easyCam.getDrag());
    settings.setValue("settings:camDistance", easyCam.getDistance());
    ofQuaternion q =  easyCam.getOrientationQuat();
    settings.setValue("settings:camQX", q.x());
    settings.setValue("settings:camQY", q.y());
    settings.setValue("settings:camQZ", q.z());
    settings.setValue("settings:camQW", q.w());

    settings.setValue("settings:p1PosX", p1Pos.x);
    settings.setValue("settings:p1PosY", p1Pos.y);
    settings.setValue("settings:p2PosX", p2Pos.x);
    settings.setValue("settings:p2PosY", p2Pos.y);
    settings.setValue("settings:p3PosX", p3Pos.x);
    settings.setValue("settings:p3PosY", p3Pos.y);
    settings.setValue("settings:p4PosX", p4Pos.x);
    settings.setValue("settings:p4PosY", p4Pos.y);
    settings.setValue("settings:calibXYPosX", calibXYPos.x);
    settings.setValue("settings:calibXYPosY", calibXYPos.y);
    settings.setValue("settings:calibSizePosX", calibSizePos.x);
    settings.setValue("settings:calibSizePosY", calibSizePos.y);
    settings.setValue("settings:calibZ", calibZ);
    settings.setValue("settings:angleX", angleX);
    settings.setValue("settings:angleY", angleY);
    settings.setValue("settings:angleZ", angleZ);
    settings.setValue("settings:dolly", dolly);
    settings.setValue("settings:frameRate", frameRate);
    settings.setValue("settings:racketThreshold", racketThreshold);
    settings.setValue("settings:maxDetectCount", maxDetectCount);
    settings.saveFile("settings.xml");

    cout << "saved" << endl;

  } else if(e.target->getLabel() == "resetGame"){
    game.reset();
    game.start();
  }
}

void ofApp::onModalEvent(ofxModalEvent e){
  if (e.type == ofxModalEvent::SHOWN)
  {
  }
  else if (e.type == ofxModalEvent::HIDDEN)
  {
    ofExit();
  }
  else if (e.type == ofxModalEvent::CONFIRM)
  {}
  else if (e.type == ofxModalEvent::CANCEL)
  {}
}

void ofApp::onToggleEvent(ofxDatGuiToggleEvent e)
{
  if (e.checked){
    if(e.target->getLabel() == "Realsense"){
      dev->start();
      cout << "start streaming" << endl;
    } else if(e.target->getLabel() == "onCalibration"){
      onCalibration = true;
    } else if(e.target->getLabel() == "onEasyCam"){
      onEasyCam = true;
    } else if(e.target->getLabel() == "enableShowCalibrationPlane"){
      enableShowCalibrationPlane = true;
    };


  } else {
    if(e.target->getLabel() == "Realsense"){
      dev->stop();
      cout << "stop streaming" << endl;
    } else if(e.target->getLabel() == "onCalibration"){
      onCalibration = false;
    } else if(e.target->getLabel() == "onEasyCam"){
      onEasyCam = false;
    } else if(e.target->getLabel() == "enableShowCalibrationPlane"){
      enableShowCalibrationPlane = false;
    };

  }
}

void ofApp::onSliderEvent(ofxDatGuiSliderEvent e)
{
  if(e.target->getLabel() == "angleX"){
    angleX = e.value;
  } else if(e.target->getLabel() == "angleY"){
    angleY = e.value;
  } else if(e.target->getLabel() == "angleZ"){
    angleZ = e.value;
  } else if(e.target->getLabel() == "calibZ"){
    calibZ = e.value;
  } else if(e.target->getLabel() == "dolly"){
    dolly = e.value;
  } else if(e.target->getLabel() == "racketThreshold"){
    racketThreshold = e.value;
  } else if(e.target->getLabel() == "maxDetectCount"){
    maxDetectCount = e.value;
  };

}


void ofApp::on2dPadEvent(ofxDatGui2dPadEvent e)
{
  if( e.target->getLabel() == "P1 position") { p1Pos.set(e.x, e.y); }
  else if ( e.target->getLabel() == "P2 position") { p2Pos.set(e.x, e.y); }
  else if ( e.target->getLabel() == "P3 position") { p3Pos.set(e.x, e.y); }
  else if ( e.target->getLabel() == "P4 position") { p4Pos.set(e.x, e.y); }
  else if ( e.target->getLabel() == "calibXY position") { calibXYPos.set(e.x, e.y);}
  else if ( e.target->getLabel() == "calibSize position") { calibSizePos.set(e.x, e.y); }
}


//--------------------------------------------------------------
cv::Mat ofApp::calcHomography(){
  cv::Point p1(p1Pos.x, p1Pos.y);
  cv::Point p2(p2Pos.x, p2Pos.y);
  cv::Point p3(p3Pos.x, p3Pos.y);
  cv::Point p4(p4Pos.x, p4Pos.y);

  cv::Point t1(0, 0);
  cv::Point t2(0, WINDOW_HEIGHT);
  cv::Point t3(WINDOW_WIDTH , WINDOW_HEIGHT);
  cv::Point t4(WINDOW_WIDTH, 0);

  std::vector<cv::Point2f> world{ p1, p2, p3, p4 };
  std::vector<cv::Point2f> scene{ t1, t2, t3, t4 };

  return cv::findHomography(world, scene, 0);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

  if( key == 27 ) // Esc
  {
    isRsDeviceReady = false;
    game.close();
    onCalibration = true;
    onEasyCam = false;
    dev->stop();
    dev->disable_stream(rs::stream::depth);
    dev->disable_stream(rs::stream::color);
    dev->disable_stream(rs::stream::infrared);
  }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
