
#include "ofxImageSequence.h"

class ofxImageSequenceLoader : public ofThread
{
  public:

	bool loading;
	bool cancelLoading;
	ofxImageSequence& sequenceRef;
	
	ofxImageSequenceLoader(ofxImageSequence* seq)
	: sequenceRef(*seq)
	, loading(true)
	, cancelLoading(false)
	{
		startThread(true);
	}
	
	~ofxImageSequenceLoader(){
        cancel();
    }
	
    void cancel(){
		if(loading){
			ofRemoveListener(ofEvents().update, this, &ofxImageSequenceLoader::updateThreadedLoad);
            lock();
			cancelLoading = true;
            unlock();
            loading = false;
			waitForThread(true);
		}
    }
    
	void threadedFunction(){
	
		ofAddListener(ofEvents().update, this, &ofxImageSequenceLoader::updateThreadedLoad);

		if(!sequenceRef.preloadAllFilenames()){
			loading = false;
			return;
		}

		if(cancelLoading){
			loading = false;
			cancelLoading = false;
			return;
		}
	
		sequenceRef.preloadAllFrames();
	
		loading = false;
	}

	void updateThreadedLoad(ofEventArgs& args){
		if(loading){
			return;
		}
		ofRemoveListener(ofEvents().update, this, &ofxImageSequenceLoader::updateThreadedLoad);

		if(sequenceRef.getTotalFrames() > 0){
			sequenceRef.completeLoading();
		}
	}

};




ofxImageSequence::ofxImageSequence()
{
	loaded = false;
	useThread = false;
	frameRate = 30.0f;
	lastFrameLoaded = -1;
	currentFrame = 0;
	maxFrames = 0;
	curLoadFrame = 0;
	threadLoader = NULL;
}

ofxImageSequence::~ofxImageSequence()
{
	unloadSequence();
}

bool ofxImageSequence::loadSequence(string prefix, string filetype,  int startDigit, int endDigit)
{
	return loadSequence(prefix, filetype, startDigit, endDigit, 0);
}

bool ofxImageSequence::loadSequence(string prefix, string filetype,  int startDigit, int endDigit, int numDigits)
{
	unloadSequence();

	char imagename[1024];
	stringstream format;
	int numFiles = endDigit - startDigit+1;
	if(numFiles <= 0 ){
		ofLogError("ofxImageSequence::loadSequence") << "No image files found.";
		return false;
	}

	if(numDigits != 0){
		format <<prefix<<"%0"<<numDigits<<"d."<<filetype;
	} else{
		format <<prefix<<"%d."<<filetype; 
	}
	
	for(int i = startDigit; i <= endDigit; i++){
		sprintf(imagename, format.str().c_str(), i);
		filenames.push_back(imagename);
		sequence.push_back(ofPixels());
		loadFailed.push_back(false);
	}
	
	loaded = true;
	
	lastFrameLoaded = -1;
	loadFrame(0);
	
	width  = sequence[0].getWidth();
	height = sequence[0].getHeight();
	return true;
}

bool ofxImageSequence::loadSequence(string _folder)
{
	unloadSequence();

	folderToLoad = _folder;

	if(useThread){
		threadLoader = new ofxImageSequenceLoader(this);
		return true;
	}

	if(preloadAllFilenames()){
		completeLoading();
		return true;
	}
	
	return false;

}

void ofxImageSequence::completeLoading()
{

	if(sequence.size() == 0){
		ofLogError("ofxImageSequence::completeLoading") << "load failed with empty image sequence";
		return;
	}

	loaded = true;	
	lastFrameLoaded = -1;
	loadFrame(0);
	
	width  = sequence[0].getWidth();
	height = sequence[0].getHeight();

}

bool ofxImageSequence::preloadAllFilenames()
{
    ofDirectory dir;
	if(extension != ""){
		dir.allowExt(extension);
	}
	
	if(!ofFile(folderToLoad).exists()){
		ofLogError("ofxImageSequence::loadSequence") << "Could not find folder " << folderToLoad;
		return false;
	}

	int numFiles;
	if(maxFrames > 0){
		numFiles = MIN(dir.listDir(folderToLoad), maxFrames);
	}
	else{	
		numFiles = dir.listDir(folderToLoad);
	}

    if(numFiles == 0) {
		ofLogError("ofxImageSequence::loadSequence") << "No image files found in " << folderToLoad;
		return false;
	}

    // read the directory for the images
	#ifdef TARGET_LINUX
	dir.sort();
	#endif


	for(int i = 0; i < numFiles; i++) {

        filenames.push_back(dir.getPath(i));
		sequence.push_back(ofPixels());
		loadFailed.push_back(false);
    }
	return true;
}

//-------------------------------------------------------------------------
void ofxImageSequence::loadEspecificFileListSequence(vector<string> fileList, int _frameRate){
	
	unloadSequence();
	
	int auxNumFiles = fileList.size();
	if(auxNumFiles <= 0)return false;
	
	filenames.clear();
	
	for(int i = 0; i < auxNumFiles; i++) {
		filenames.push_back(fileList[i]);
		sequence.push_back(ofPixels());
		loadFailed.push_back(false);
		
		//Load the image without threads
		curLoadFrame = i;
		if(!ofLoadImage(sequence[i], filenames[i])){
			loadFailed[i] = true;
			ofLogError("ofxImageSequence::loadFrame") << "My Image failed to load: " << filenames[i];
		}
	}
	
	loaded = true;
	
	lastFrameLoaded = -1;
	loadFrame(0);
	
	width  = sequence[0].getWidth();
	height = sequence[0].getHeight();
	
	frameRate = _frameRate;

}

//-------------------------------------------------------------------------
void ofxImageSequence::ofxImageSequence::drawCoverFlow(int deltaXImgs, int space, int posYCoverFlow, float scaleX, float scaleY){
	
	for (int i = 0; i < sequence.size(); i++){
		ofTexture myText;
		myText.loadData(sequence[i]);
		myText.draw((i*(sequence[i].getWidth()+space)+deltaXImgs), posYCoverFlow, myText.getWidth()*scaleX, myText.getWidth()*scaleY );
	}
}

//-------------------------------------------------------------------------
//set to limit the number of frames. negative means no limit
void ofxImageSequence::setMaxFrames(int newMaxFrames)
{
	maxFrames = MAX(newMaxFrames, 0);
	if(loaded){
		ofLogError("ofxImageSequence::setMaxFrames") << "Max frames must be called before load";
	}
}

void ofxImageSequence::setExtension(string ext)
{
	extension = ext;
}

void ofxImageSequence::enableThreadedLoad(bool enable){

	if(loaded){
		ofLogError("ofxImageSequence::enableThreadedLoad") << "Need to enable threaded loading before calling load";
	}
	useThread = enable;
}

void ofxImageSequence::cancelLoad()
{
	if(useThread && threadLoader != NULL){
        threadLoader->cancel();
        
		delete threadLoader;
		threadLoader = NULL;
	}
}

void ofxImageSequence::setMinMagFilter(int newMinFilter, int newMagFilter)
{
	minFilter = newMinFilter;
	magFilter = newMagFilter;
	texture.setTextureMinMagFilter(minFilter, magFilter);
}

void ofxImageSequence::preloadAllFrames()
{
	if(sequence.size() == 0){
		ofLogError("ofxImageSequence::loadFrame") << "Calling preloadAllFrames on unitialized image sequence.";
		return;
	}
	
	for(int i = 0; i < sequence.size(); i++){
		//threaded stuff
		if(useThread){
			if(threadLoader == NULL){
				return;
			}
            threadLoader->lock();
            bool shouldExit = threadLoader->cancelLoading;
            threadLoader->unlock();
            if(shouldExit){
                return;
            }

			ofSleepMillis(15);
		}
		curLoadFrame = i;
		if(!ofLoadImage(sequence[i], filenames[i])){
			loadFailed[i] = true;
			ofLogError("ofxImageSequence::loadFrame") << "Image failed to load: " << filenames[i];		
		}
	}
}

float ofxImageSequence::percentLoaded(){
	if(isLoaded()){
		return 1.0;
	}
	if(isLoading() && sequence.size() > 0){
		return 1.0*curLoadFrame / sequence.size();
	}
	return 0.0;
}

void ofxImageSequence::loadFrame(int imageIndex)
{
	if(lastFrameLoaded == imageIndex){
		return;
	}

	if(imageIndex < 0 || imageIndex >= sequence.size()){
		ofLogError("ofxImageSequence::loadFrame") << "Calling a frame out of bounds: " << imageIndex;
		return;
	}

	if(!sequence[imageIndex].isAllocated() && !loadFailed[imageIndex]){
		if(!ofLoadImage(sequence[imageIndex], filenames[imageIndex])){
			loadFailed[imageIndex] = true;
			ofLogError("ofxImageSequence::loadFrame") << "Image failed to load: " << filenames[imageIndex];
		}
	}

	if(loadFailed[imageIndex]){
		return;
	}

	texture.loadData(sequence[imageIndex]);

	lastFrameLoaded = imageIndex;

}

float ofxImageSequence::getPercentAtFrameIndex(int index)
{
	return ofMap(index, 0, sequence.size()-1, 0, 1.0, true);
}

float ofxImageSequence::getWidth()
{
	return width;
}

float ofxImageSequence::getHeight()
{
	return height;
}

void ofxImageSequence::unloadSequence()
{
	if(threadLoader != NULL){
		delete threadLoader;
		threadLoader = NULL;
	}

	sequence.clear();
	filenames.clear();
	loadFailed.clear();

	loaded = false;
	width = 0;
	height = 0;
	curLoadFrame = 0;
	lastFrameLoaded = -1;
	currentFrame = 0;	

}

void ofxImageSequence::setFrameRate(float rate)
{
	frameRate = rate;
}

string ofxImageSequence::getFilePath(int index){
	if(index > 0 && index < filenames.size()){
		return filenames[index];
	}
	ofLogError("ofxImageSequence::getFilePath") << "Getting filename outside of range";
	return "";
}

int ofxImageSequence::getFrameIndexAtPercent(float percent)
{
    if (percent < 0.0 || percent > 1.0) percent -= floor(percent);

	return MIN((int)(percent*sequence.size()), sequence.size()-1);
}

//deprecated
ofTexture& ofxImageSequence::getTextureReference()
{
	return getTexture();
}

//deprecated
ofTexture* ofxImageSequence::getFrameAtPercent(float percent)
{
	setFrameAtPercent(percent);
	return &getTexture();
}

//deprecated
ofTexture* ofxImageSequence::getFrameForTime(float time)
{
	setFrameForTime(time);
	return &getTexture();
}

//deprecated
ofTexture* ofxImageSequence::getFrame(int index)
{
	setFrame(index);
	return &getTexture();
}

ofTexture& ofxImageSequence::getTextureForFrame(int index)
{
	setFrame(index);
	return getTexture();
}

ofTexture& ofxImageSequence::getTextureForTime(float time)
{
	setFrameForTime(time);
	return getTexture();
}

ofTexture& ofxImageSequence::getTextureForPercent(float percent){
	setFrameAtPercent(percent);
	return getTexture();
}

void ofxImageSequence::setFrame(int index)
{
	if(!loaded){
		ofLogError("ofxImageSequence::setFrame") << "Calling getFrame on unitialized image sequence.";
		return;
	}

	if(index < 0){
		ofLogError("ofxImageSequence::setFrame") << "Asking for negative index.";
		return;
	}
	
	index %= getTotalFrames();
	
	loadFrame(index);
	currentFrame = index;
}

void ofxImageSequence::setFrameForTime(float time)
{
	float totalTime = sequence.size() / frameRate;
	float percent = time / totalTime;
	return setFrameAtPercent(percent);	
}

void ofxImageSequence::setFrameAtPercent(float percent)
{
	setFrame(getFrameIndexAtPercent(percent));	
}

ofTexture& ofxImageSequence::getTexture()
{
	return texture;
}

const ofTexture& ofxImageSequence::getTexture() const
{
	return texture;
}

float ofxImageSequence::getLengthInSeconds()
{
	return getTotalFrames() / frameRate;
}

int ofxImageSequence::getTotalFrames()
{
	return sequence.size();
}

bool ofxImageSequence::isLoaded(){						//returns true if the sequence has been loaded
    return loaded;
}

bool ofxImageSequence::isLoading(){
	return threadLoader != NULL && threadLoader->loading;
}
