#include "game.h"


Game::~Game(){}

void Game::setup(float frameRate, int width, int height) {
  
  WINDOW_WIDTH = width;
  WINDOW_HEIGHT = height;
  WIN_POINT = 5;
  frameRate = frameRate;
  
  // font settings
  ofTrueTypeFont::setGlobalDpi(72);
  font.load("fonts/Verdana.ttf", 80); // フォントデータを指定する
  
  // Box2d settings.
  box2d.init();
  box2d.enableEvents();
  box2d.setGravity(0, 0);
  box2d.createBounds();
  box2d.setFPS((float)frameRate);
  box2d.registerGrabbing();
  box2d.getWorld()->SetAllowSleeping(true);
  box2d.getWorld()->SetContinuousPhysics(true);
  ofAddListener(box2d.contactStartEvents, this, &Game::contactStart);
  ofAddListener(box2d.contactEndEvents, this, &Game::contactEnd);
  
  // game settings
  start();
  
  // ラケット
  for( int i = 0; i < RACKET_COUNT; i++ ){
    shared_ptr<RacketPoly> poly = shared_ptr<RacketPoly>(new RacketPoly);
    poly.get()->setPhysics(1.0f, 1.0f, 1.0f);
    poly.get()->setData(new BodyData(i, "racket"));
    poly.get()->create(box2d.getWorld());
    rackets.push_back(poly);
  }
  
  // ゴール
  shared_ptr<ofxBox2dRect> goal = shared_ptr<ofxBox2dRect>(new ofxBox2dRect);
  shared_ptr<ofxBox2dRect> goal2 = shared_ptr<ofxBox2dRect>(new ofxBox2dRect);
  int goalTickness = 20;
  goal->setup(box2d.getWorld(), 0, 0, goalTickness, WINDOW_HEIGHT * 2);
  goal->setData(new BodyData(333, "goal1"));
  goal2->setup(box2d.getWorld(), WINDOW_WIDTH, 0, goalTickness, WINDOW_HEIGHT * 2);
  goal2->setData(new BodyData(444, "goal2"));
  goals.push_back(goal);
  goals.push_back(goal2);
  
  
  // 得点を初期化
  player1Point = 0;
  player2Point = 0;
  
  // 効果音を読み込む
  struckSound.load("sounds/struck.mp3");   // ラケットとボールが接触した時の音声
  goalSound.load("sounds/goal.mp3");       // ゴールにボールが入った時の音声
  resetSound.load("sounds/reset.mp3");     // リセットボタンを押下した時の音声
  gameSetSound.load("sounds/gameset.mp3"); // ゲームセット時の音声
  goalSound.setVolume(1);
  struckSound.setVolume(1);
  resetSound.setVolume(1);
  gameSetSound.setVolume(1);
  
  // ゴール時の画像を読み込む
  p1GoalImg.load("images/p1_goal.png");
  p2GoalImg.load("images/p2_goal.png");
  
}

void Game::update(std::vector<std::vector<cv::Point>> contours, float racketThreshold, float maxDetectCount){
  
  box2d.update();
  
  auto maxArea = 200000.0;
  int count = 0;
  bool player1RacketDetected = false;
  bool player2RacketDetected = false;
  int cs = 0;
  
  
  for(auto cnt : contours)
  {
    if(count < RACKET_COUNT && cs < maxDetectCount){
      
      auto area = cv::contourArea(cnt);
      cv::RotatedRect rect =  cv::minAreaRect(cnt);
      cv::Point2f vertices[4];
      rect.points(vertices);
      if(area > 200000) continue;
      if(area < maxArea && area > racketThreshold){
        if(isPlayer2Side(vertices) && !player2RacketDetected)
        {
          createRacket(vertices, count, 2);
          ++count;
          player2RacketDetected = true;
        }
        
        if(isPlayer1Side(vertices) && !player1RacketDetected)
        {
          createRacket(vertices, count, 1);
          ++count;
          player1RacketDetected = true;
        }
        
        
      }
      cs++;
    } else {
      break;
    }
    
  }
  
  if(player1Point >= WIN_POINT) {
    isPlayer1Win = true;
    isPlayer1Goaled = false;
    isPlayer2Goaled = false;
    circle.get()->isDead =true;
    createReset();
  }
  
  if(player2Point >= WIN_POINT) {
    isPlayer2Win = true;
    isPlayer1Goaled = false;
    isPlayer2Goaled = false;
    circle.get()->isDead =true;
    createReset();
  }
  
  if(!player1RacketDetected && !player2RacketDetected ){
    for(auto r : rackets) {
      if(r.get()->size() > 0 && !box2d.world->IsLocked()) r.get()->clear();
    }
  }
  
  
}

void Game::draw(){
  
  ofBackground(COLOR_BLACK);
  ofSetColor(COLOR_WHITE);
  if(isPlayer1Win || isPlayer2Win) ofSetColor(COLOR_BLACK);
  ofSetLineWidth(20);
  ofDrawLine(WINDOW_WIDTH/2, 0, WINDOW_WIDTH/2, WINDOW_HEIGHT);
  
  ofSetColor(COLOR_WHITE);
  
  if(circle.get()->isDead) {
    if(!box2d.world->IsLocked()){
      circle.get()->destroy();
      ofPoint initialPos;
      if(isPlayer1Goaled){
        initialPos.x = WINDOW_WIDTH/2 + 50;
        initialPos.y = WINDOW_HEIGHT/2;
      }
      else if(isPlayer2Goaled){
        initialPos.x = WINDOW_WIDTH/2 - 50;
        initialPos.y = WINDOW_HEIGHT/2;
      }
      else{
        initialPos.x = WINDOW_WIDTH/2;
        initialPos.y = WINDOW_HEIGHT/2;
      }
      
      createBall(initialPos);
    }
  } else {
    circle.get()->draw();
  }
  
  
  
  ofSetColor(COLOR_WHITE);
  for(auto r : rackets){
    if(r.get()->size() > 0){
      r.get()->create(box2d.getWorld());
      r.get()->setData(new BodyData(r->player, "racket"));
      r.get()->draw();
    }
  }
  
  
  for(auto g : goals){
    g->draw();
  };
  
  
  // 得点を描画
  ofSetColor(COLOR_WHITE);
  ofPushMatrix();
  ofTranslate(WINDOW_WIDTH/2 + 70, WINDOW_HEIGHT - 70);
  ofRotate(180);
  font.drawString(to_string(player1Point), 100, 0);
  font.drawString(to_string(player2Point), 0, 0);
  ofPopMatrix();
  
  if(isPlayer1Goaled){
    p1GoalImg.draw(0, 0);
    goalFrameCount++;
    if(goalFrameCount > 60 / 2){
      goalFrameCount = 0;
      isPlayer1Goaled = false;
    }
  }
  
  if(isPlayer2Goaled){
    p2GoalImg.draw(WINDOW_WIDTH  * 3 / 4, 0);
    goalFrameCount++;
    if(goalFrameCount > 60 / 2){
      goalFrameCount = 0;
      isPlayer2Goaled = false;
    }
  }
  
  if(isPlayer1Win){
    ofPushMatrix();
    ofTranslate(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
    ofRotate(180);
    font.drawString("Player1 win !!", -WINDOW_WIDTH/3 - 60, 0);
    ofPopMatrix();
    if(resetButton.get()->isDead){
      if(!box2d.world->IsLocked()){
        resetButton.get()->destroy();
        isPlayer1Win = false;
        isPlayer2Win = false;
        resetBall();
      }
    } else {
      resetButton.get()->draw();
    }
    
  }
  
  if(isPlayer2Win){
    ofPushMatrix();
    ofTranslate(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
    ofRotate(180);
    font.drawString("Player2 win !!", -WINDOW_WIDTH/3 - 60, 0);
    ofPopMatrix();
    if(resetButton.get()){
      
      
      if(resetButton.get()->isDead){
        if(!box2d.world->IsLocked()){
          resetButton.get()->destroy();
          isPlayer1Win = false;
          isPlayer2Win = false;
          resetBall();
        }
      } else {
        resetButton.get()->draw();
      }
    }
  }
  
}

void Game::start(){
  isPlayer1Win = false;
  isPlayer2Win = false;
  ofPoint initialPos;
  initialPos.x = WINDOW_WIDTH/2;
  initialPos.y = WINDOW_HEIGHT/2;
  createBall(initialPos);
}


void Game::reset(){
  player1Point = 0;
  player2Point = 0;
}


void Game::close() {
  // TODO
}


//--------------------------------------------------------------
void Game::contactStart(ofxBox2dContactArgs &e) {
  
  // TODO: 音を再生する処理をcontactEndからここへ移行する。
}

//--------------------------------------------------------------
void Game::contactEnd(ofxBox2dContactArgs &e) {
  
  if(e.a != NULL && e.b != NULL) {
    b2Body *bodyA = e.a->GetBody();
    b2Body *bodyB = e.b->GetBody();
    
    BodyData * dataA = (BodyData *)bodyA->GetUserData();
    BodyData * dataB = (BodyData *)bodyB->GetUserData();
    
    if(dataA != NULL)
    {
      string name = dataA->name;
      if(name == "circle"){
        if(dataB){
          
          if(dataB->name == "racket"){
            
            struckSound.play();
            
            float angle = bodyB->GetAngle() * RAD_TO_DEG;
            float force_x = 5.0f;
            
            if(dataB->id != 1){ //p2 racket
              angle = -angle;
              force_x = -force_x;
            }
            
            b2Vec2 acc = b2Vec2(force_x, angle);
            bodyA->SetLinearVelocity(bodyA->GetLinearVelocity() + acc);
          }
        }
      }
      else if(name == "goal1")
      {
        if(dataB){
          if(dataB->name == "circle"){
            isPlayer2Goaled = true;
            ++player2Point;
            if(player2Point >= WIN_POINT) {
              gameSetSound.play();
            } else {
              goalSound.play();
              
            }
            
            resetBall();
            
            
          }
        }
      }
      else if(name == "goal2")
      {
        if(dataB){
          if(dataB->name == "circle"){
            isPlayer1Goaled = true;
            ++player1Point;
            if(player1Point >= WIN_POINT) {
              gameSetSound.play();
            } else {
              goalSound.play();
              
            }            resetBall();
          }
        }
      } else if(name == "racket")
      {
        if(dataB){
          if(dataB->name == "reset"){
            resetSound.play();
            resetButton.get()->isDead = true;
            reset();
          }
        }
      }
      else {
        //何もしない。
      }
    }
    
    if(dataB != NULL){
      string name = dataB->name;
      
      if(name == "circle"){
        if(dataA){
          
          if(dataA->name == "racket"){
            
            struckSound.play();
            
            float angle = bodyA->GetAngle() * RAD_TO_DEG;
            float force_x = 5.0f;
            
            if(dataA->id != 1){ //p2 racket
              angle = -angle;
              force_x = -force_x;
            }
            
            b2Vec2 acc = b2Vec2(force_x, angle);
            bodyB->SetLinearVelocity(bodyB->GetLinearVelocity() + acc);
          }
        }
      }
      else if(name == "goal1")
      {
        if(dataA){
          if(dataA->name == "circle"){
            isPlayer2Goaled = true;
            ++player2Point;
            if(player2Point >= WIN_POINT) {
              gameSetSound.play();
            } else {
              goalSound.play();
              
            }
            resetBall();
            
          }
        }
      }
      else if(name == "goal2")
      {
        if(dataA){
          if(dataA->name == "circle"){
            isPlayer1Goaled = true;
            ++player1Point;
            if(player1Point >= WIN_POINT) {
              gameSetSound.play();
            } else {
              goalSound.play();
              
            }            resetBall();
          }
        }
      } else if(name == "racket")
      {
        if(dataA){
          if(dataA->name == "reset"){
            resetSound.play();
            resetButton.get()->isDead = true;
            reset();
          }
        }
        
      }
      else {
        //何もしない。
      }
    }
    
    
  }
  
  
}

bool Game::createBall(ofPoint initialPos){
  if(!box2d.getWorld()->IsLocked()){
    circle = shared_ptr<textureCircle>(new textureCircle);
    float rad = 25.0f;
    circle.get()->setPhysics(1.0f, 1.0f, 1.0f);
    circle.get()->setupTexture();
    circle.get()->setup(box2d.getWorld(), initialPos.x, initialPos.y, rad);
    circle.get()->setData(new BodyData(999, "circle"));
    
    return true;
  }
}

bool Game::destroyBall(b2Body *body){
  
}

bool Game::resetBall(){
  circle.get()->isDead = true;
}


void Game::createRacket(cv::Point2f vertices[4], int count, int player){
  if(rackets[count].get()->size() > 0){
    rackets[count].get()->clear();
    rackets[count].get()->player = player;
  }
  
  rackets[count].get()->addVertex(vertices[0].x, vertices[0].y);
  rackets[count].get()->addVertex(vertices[1].x, vertices[1].y);
  rackets[count].get()->addVertex(vertices[2].x, vertices[2].y);
  rackets[count].get()->addVertex(vertices[3].x, vertices[3].y);
  rackets[count].get()->close();
  
}

void Game::createReset(){
  resetButton = shared_ptr<resetRect>(new resetRect);
  resetButton.get()->setPhysics(.0f, .0f, .0f);
  resetButton.get()->setupTexture();
  resetButton.get()->isDead = false;
  resetButton.get()->setup(box2d.getWorld(), WINDOW_WIDTH/2, 50, 100, 100);
  resetButton.get()->setData(new BodyData(222, "reset"));
  
}


// ラケットの位置がプレイヤー１サイドにあるか？
// TODO: RacketPolyクラスに移行する。
bool Game::isPlayer1Side(cv::Point2f verts[4]){
  for(auto i = 0; i < 4; i++){
    if(WINDOW_WIDTH/2 + 30 <= verts[i].x){
      return false;
    }
  }
  return true;
}

// ラケットの位置がプレイヤー２サイドにあるか？
// TODO: RacketPolyクラスに移行する。
bool Game::isPlayer2Side(cv::Point2f verts[4]){
  for(auto i = 0; i < 4; i++){
    if(WINDOW_WIDTH/2 - 30 > verts[i].x){
      return false;
    }
  }
  return true;
}
