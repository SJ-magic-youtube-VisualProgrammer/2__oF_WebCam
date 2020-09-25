/************************************************************
************************************************************/
#include "ofApp.h"

/************************************************************
************************************************************/

/******************************
******************************/
ofApp::ofApp(string _str_CamName)
: str_CamName(_str_CamName)
{
	font.load("font/RictyDiminished-Regular.ttf", 15, true, true, true);
	
	fp = fopen("../../../data/Log.csv", "w");
}

/******************************
******************************/
ofApp::~ofApp(){
	fclose(fp);
	
	printf("Good-bye\n");
	fflush(stdout);
}

/******************************
******************************/
void ofApp::setup(){
	/********************
	********************/
	ofSetWindowTitle("WebCam");
	
	ofSetWindowShape( WINDOW_WIDTH, WINDOW_HEIGHT );
	ofSetVerticalSync(true);
	ofSetFrameRate(FrameRate);
	
	ofSetEscapeQuitsApp(false);
	
	/********************
	********************/
	setup_Camera();
}

/******************************
******************************/
void ofApp::setup_Camera()
{
	/********************
	********************/
	printf("> setup camera\n");
	fflush(stdout);
	
	/********************
	細かく情報出してもらう
	********************/
	ofSetLogLevel(OF_LOG_VERBOSE);
	for(int i = 0; i < NUM_CAMS; i++)	CamSet[i].cam.setVerbose(true);
	
	vector< ofVideoDevice > Devices = CamSet[0].cam.listDevices();
	
	/********************
	********************/
	if(str_CamName == "") { ofExit(); return; }
	
	/********************
	search for camera by device name.
	********************/
	int Grabber_id = 0;
	for(int i = 0; i < Devices.size(); i++){
		if(Devices[i].deviceName.find(str_CamName) != std::string::npos){
			CamSet[Grabber_id].cam.setDeviceID(i);
			
			printf("---------- CamSet %d ----------\n", Grabber_id);
			printf("> cam set device id = %d\n", i);
			printf("> DeviceID     = %d\n", Devices[i].id);
			printf("> CamName      = %s\n", Devices[i].deviceName.c_str());
			printf("> HardwareName = %s\n", Devices[i].hardwareName.c_str());
			printf("> serialID     = %s\n", Devices[i].serialID.c_str());
			
			printf( "\n> Cam size asked  = (%d, %d)\n", int(CAM_WIDTH), int(CAM_HEIGHT) );
			/*
				https://openframeworks.cc/documentation/video/ofVideoGrabber/#show_setDesiredFrameRate
					Set's the desired frame rate of the grabber. This should be called before initGrabber(), which will try to initialize at the desired frame rate. 
					Not all frame rates will be supported, but this at least gives you some abilitity to try grab at different rates.
			*/
			CamSet[Grabber_id].cam.setDesiredFrameRate(30);
			CamSet[Grabber_id].cam.initGrabber(CAM_WIDTH, CAM_HEIGHT);
			printf( "> Cam size actual = (%d, %d)\n\n", int(CamSet[Grabber_id].cam.getWidth()), int(CamSet[Grabber_id].cam.getHeight()) );
			fflush(stdout);
			
			/* */
			fbo[Grabber_id].allocate(int(CamSet[Grabber_id].cam.getWidth()), int(CamSet[Grabber_id].cam.getHeight()), GL_RGBA);
			CamSet[Grabber_id].p_fbo = &fbo[Grabber_id];
			
			Grabber_id++;
			
			if(NUM_CAMS <= Grabber_id) break;
		}
	}
	
	/********************
	Grabber_id shows num of Cams found here.
	********************/
	if(Grabber_id < NUM_CAMS)	b_CamSearchFailed = true;
}

/******************************
******************************/
void ofApp::update(){
	/********************
	********************/
	now = ofGetElapsedTimeMillis();
	
	/********************
	********************/
	if(b_CamSearchFailed){
		if(2000 < now)	{ ofExit(1); return; }
		else			{ return; }
	}
	
	/********************
	********************/
	for(int i = 0; i < NUM_CAMS; i++){
		CamSet[i].cam.update();
		if(CamSet[i].cam.isFrameNew()){
			update_fbo_by_Cam(CamSet[i]);
			
			float Current_Cam_FrameRate = 1000.0/(float)(now - CamSet[i].t_LastCamUpdate);
			float alpha = 0.03;
			CamSet[i].Cam_FrameRate = alpha * Current_Cam_FrameRate + (1 - alpha) * CamSet[i].Cam_FrameRate;
			
			CamSet[i].t_LastCamUpdate = now;
		}
	}
}

/******************************
******************************/
void ofApp::update_fbo_by_Cam(CAM_SET& CamSet){
	/********************
	********************/
	ofEnableAlphaBlending();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	// ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnableSmoothing();
	
	/********************
	********************/
	CamSet.p_fbo->begin();
		/********************
		********************/
		ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 255);
		CamSet.cam.draw(0, 0, CamSet.p_fbo->getWidth(), CamSet.p_fbo->getHeight());
		
		/********************
		********************/
		ofSetColor(0, 100, 255, 255);
		
		char buf[512];
		sprintf(buf, "Cam : %5.1f", CamSet.Cam_FrameRate);
		font.drawString(buf, 10, 30);
	CamSet.p_fbo->end();
}

/******************************
******************************/
void ofApp::draw(){
	/********************
	********************/
	if(b_CamSearchFailed){ drawMessage_CamSearchFailed(); return; }
	
	/********************
	********************/
	ofBackground(0);
	ofSetColor(255, 255, 255, 255);
	for(int i = 0; i < NUM_CAMS; i++){
		fbo[i].draw(fbo[0].getWidth() * i, 0);
	}
}

/******************************
******************************/
void ofApp::drawMessage_CamSearchFailed(){
	ofBackground(0);
	ofSetColor(255, 0, 0, 255);
	
	char buf[512];
	sprintf(buf, "USB Camera not Exsist");
	font.drawString(buf, ofGetWidth()/2 - font.stringWidth(buf)/2, ofGetHeight()/2);
}

/******************************
******************************/
void ofApp::keyPressed(int key){
	switch(key){
		case ' ':
			{
				/********************
				slide
				********************/
				ofFbo* temp = CamSet[0].p_fbo;
				
				for(int i = 0; i < NUM_CAMS - 1; i++){
					CamSet[i].p_fbo = CamSet[i + 1].p_fbo;
				}
				CamSet[NUM_CAMS - 1].p_fbo = temp;
				
				printf("slide ok\n");
				fflush(stdout);
			}
			break;
	}
}

/******************************
******************************/
void ofApp::keyReleased(int key){

}

/******************************
******************************/
void ofApp::mouseMoved(int x, int y ){

}

/******************************
******************************/
void ofApp::mouseDragged(int x, int y, int button){

}

/******************************
******************************/
void ofApp::mousePressed(int x, int y, int button){

}

/******************************
******************************/
void ofApp::mouseReleased(int x, int y, int button){

}

/******************************
******************************/
void ofApp::mouseEntered(int x, int y){

}

/******************************
******************************/
void ofApp::mouseExited(int x, int y){

}

/******************************
******************************/
void ofApp::windowResized(int w, int h){

}

/******************************
******************************/
void ofApp::gotMessage(ofMessage msg){

}

/******************************
******************************/
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
