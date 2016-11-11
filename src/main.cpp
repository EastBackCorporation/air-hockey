#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
  ofGLWindowSettings settings;
  
  // Main Screen/Calibration
  settings.width = 640;//320; //640; 830/750 400/361
  settings.height = 480;
  settings.setPosition(ofVec2f(420,0));
  auto mainWindow = ofCreateWindow(settings);
  
  // GUI
  settings.width = 640;
  settings.height = 800;
  settings.setPosition(ofVec2f(0,0));
  auto guiWindow = ofCreateWindow(settings);
  guiWindow->setVerticalSync(false);
  
  auto mainApp = make_shared<ofApp>();
  mainApp->setupGui();
  
  ofAddListener(guiWindow->events().update, mainApp.get(), &ofApp::updateGui);
  ofAddListener(guiWindow->events().draw, mainApp.get(), &ofApp::drawGui);
  ofRunApp(mainWindow, mainApp);
  ofRunMainLoop();
}
