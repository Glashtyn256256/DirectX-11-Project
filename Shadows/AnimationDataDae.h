#ifndef ANIMATION_DATA_DAE_H
#define ANIMATION_DATA_DAE_H

#include "Application.h"
#include "tinyxml2.h"
#include <map>

class SkeletonAnimationData {
public:
	SkeletonAnimationData();



	std::string boneName; //getboneName

	std::vector <float> tranTime;
	std::vector<XMFLOAT4> translate;

	std::vector<float> rotTime;
	std::vector<XMFLOAT4> rotations;
	 
	//std::vector<float> rotX;
	//std::vector<float> rotY;
	//std::vector<float> rotZ;

	void ResetCurrentFrames() 
	{
		tranCurrentFrame = 0;
		rotCurrentFrame = 0;
	}
	int tranCurrentFrame; //Where the 
	int rotCurrentFrame;

	float previousRotationTime;
	float previousTranslationTime;

};

class AnimationDataDae
{
public:
	AnimationDataDae(const char* filePath);
	~AnimationDataDae();
	//when you're given a key link it to the skeletonanimationdata
	std::map<std::string,SkeletonAnimationData*> boneAnimation;
	void readDaeFile(const char* filePath);
	float endTime;
private:

};




#endif