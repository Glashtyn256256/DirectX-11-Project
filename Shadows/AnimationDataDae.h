#ifndef ANIMATION_DATA_DAE_H
#define ANIMATION_DATA_DAE_H

#include "Application.h"
#include "tinyxml2.h"
#include <map>

class SkeletonAnimationData {
public:
	SkeletonAnimationData();



	std::string boneName; //getboneName

	std::vector <float> translationTimes;
	std::vector<XMFLOAT4> translation;

	std::vector<float> rotationTimes;
	std::vector<XMFLOAT4> rotations;
	 
	//std::vector<float> rotX;
	//std::vector<float> rotY;
	//std::vector<float> rotZ;

	void ResetCurrentFrames() 
	{
		translationCurrentFrame = 0;
		rotationCurrentFrame = 0;
	}
	void ResetPreviousTimes()
	{
		previousTranslationTime = 0;
		previousRotationTime = 0;
	}
	
	int translationCurrentFrame; //Where the 
	int rotationCurrentFrame;

	float previousRotationTime;
	float previousTranslationTime;

	//this slows down the attack animation when it's private no.

	/*int GetTranslationCurrentFrame() { return translationCurrentFrame; }
	void SetTranslationCurrentFrame(int frame) { translationCurrentFrame = frame; }

	int GetRotationCurrentFrame() { return rotationCurrentFrame; }
	void SetRotationCurrentFrame(int frame) { rotationCurrentFrame = frame; }

	float GetPreviousTranslationTime() { return previousTranslationTime; }
	void SetPreviousTranslationTime(int previoustime) { previousTranslationTime = previoustime; }

	float GetPreviousRotationTime() { return previousRotationTime; }
	void SetPreviousRotationTime(int previoustime) { previousRotationTime = previoustime; }*/

//private:
//	int translationCurrentFrame; //Where the 
//	int rotationCurrentFrame;
//
//	float previousRotationTime;
//	float previousTranslationTime;

};

class AnimationDataDae
{
public:
	AnimationDataDae(const char* filePath);
	~AnimationDataDae();
	//when you're given a key link it to the skeletonanimationdata
	std::map<std::string,SkeletonAnimationData*> boneAnimation;
	void readDaeFile(const char* filePath);
	float finalFrameEndTime;
private:

};




#endif