#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( int argc, char** argv ){
	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context

	/********************
	********************/
	string str_CamName = "";
	
	printf("---------------------------------\n");
	printf("> parameters\n");
	printf("\t-c camera_Name\n");
	printf("---------------------------------\n");
	
	for(int i = 1; i < argc; i++){
		if( strcmp(argv[i], "-c") == 0 ){
			if(i+1 < argc) str_CamName = argv[i + i];
		}
	}
	
	ofRunApp(new ofApp(str_CamName));
}
