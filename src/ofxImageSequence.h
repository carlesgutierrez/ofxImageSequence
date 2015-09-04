
#pragma once

#include "ofMain.h"

class ofxImageSequenceLoader;
class ofxImageSequence : public ofBaseHasTexture {
public:
	
	ofxImageSequence();
	~ofxImageSequence();
	
	//sets an extension, like png or jpg
	void setExtension(string prefix);
	void setMaxFrames(int maxFrames); //set to limit the number of frames. 0 or less means no limit
	void enableThreadedLoad(bool enable);
	
	/**
	 *	use this method to load sequences formatted like:
	 *	path/to/images/myImage8.png
	 *	path/to/images/myImage9.png
	 *	path/to/images/myImage10.png
	 *
	 *	for this sequence the parameters would be:
	 *	prefix		=> "path/to/images/myImage"
	 *	filetype	=> "png"
	 *	startIndex	=> 8
	 *	endIndex	=> 10
	 */
	bool loadSequence(string prefix, string filetype, int startIndex, int endIndex);
	
	/**
	 *	Use this function to load sequences formatted like
	 *
	 *	path/to/images/myImage004.jpg
	 *	path/to/images/myImage005.jpg
	 *	path/to/images/myImage006.jpg
	 *	path/to/images/myImage007.jpg
	 *
	 *	for this sequence the parameters would be:
	 *	prefix		=> "path/to/images/myImage"
	 *	filetype	=> "jpg"
	 *	startIndex	=> 4
	 *	endIndex	=> 7
	 *	numDigits	=> 3
	 */
	bool loadSequence(string prefix, string filetype, int startIndex, int endIndex, int numDigits);
	bool loadSequence(string folder);
	
	void cancelLoad();
	void preloadAllFrames();		//immediately loads all frames in the sequence, memory intensive but fastest scrubbing
	void unloadSequence();			//clears out all frames and frees up memory
	
	void setFrameRate(float rate); //used for getting frames by time, default is 30fps
	
	//these get textures, but also change the
	OF_DEPRECATED_MSG("Use getTextureForFrame instead.",   ofTexture* getFrame(int index));		 //returns a frame at a given index
	OF_DEPRECATED_MSG("Use getTextureForTime instead.",    ofTexture* getFrameForTime(float time)); //returns a frame at a given time, used setFrameRate to set time
	OF_DEPRECATED_MSG("Use getTextureForPercent instead.", ofTexture* getFrameAtPercent(float percent)); //returns a frame at a given time, used setFrameRate to set time
	
	ofTexture& getTextureForFrame(int index);		 //returns a frame at a given index
	ofTexture& getTextureForTime(float time); //returns a frame at a given time, used setFrameRate to set time
	ofTexture& getTextureForPercent(float percent); //returns a frame at a given time, used setFrameRate to set time
	
	//if usinsg getTextureRef() use these to change the internal state
	void setFrame(int index);
	void setFrameForTime(float time);
	void setFrameAtPercent(float percent);
	
	string getFilePath(int index);
	
	OF_DEPRECATED_MSG("Use getTexture() instead.", ofTexture& getTextureReference());
	
	virtual ofTexture& getTexture();
	virtual const ofTexture& getTexture() const;
	
	virtual void setUseTexture(bool bUseTex){/* not used */};
	virtual bool isUsingTexture() const{return true;}
	
	int getFrameIndexAtPercent(float percent);	//returns percent (0.0 - 1.0) for a given frame
	float getPercentAtFrameIndex(int index);	//returns a frame index for a percent
	
	int getCurrentFrame(){ return currentFrame; };
	int getTotalFrames();					//returns how many frames are in the sequence
	float getLengthInSeconds();				//returns the sequence duration based on frame rate
	
	float getWidth();						//returns the width/height of the sequence
	float getHeight();
	bool isLoaded();						//returns true if the sequence has been loaded
	bool isLoading();						//returns true if loading during thread
	void loadFrame(int imageIndex);			//allows you to load (cache) a frame to avoid a stutter when loading. use this to "read ahead" if you want
	
	void setMinMagFilter(int minFilter, int magFilter);
	
	//Do not call directly
	//called internally from threaded loader
	void completeLoading();
	bool preloadAllFilenames();		//searches for all filenames based on load input
	float percentLoaded();
	
	/////// WIP
	//Added specific sequences list file names with folder included
	bool loadEspecificFileListSequence(vector<string> fileList, int _frameRate);
	string getFolderLoaded();
	void drawCoverFlow(int initX, int deltaXImgs, int posYCoverFlow, int space,  float scaleX, float scaleY);
	//////
	
protected:
	ofxImageSequenceLoader* threadLoader;
	
	vector<ofPixels> sequence;
	vector<string> filenames;
	vector<bool> loadFailed;
	int currentFrame;
	ofTexture texture;
	string extension;
	
	string folderToLoad;
	int curLoadFrame;
	int maxFrames;
	bool useThread;
	bool loaded;
	
	float width, height;
	int lastFrameLoaded;
	float frameRate;
	
	int minFilter;
	int magFilter;
};
