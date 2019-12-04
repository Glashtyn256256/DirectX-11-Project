#include "AnimationDataDae.h"

#include <iostream>
#include <String>
#include <fstream>
#include <vector>
#include <algorithm> // need for std::replace that erase uses
#include <sstream> // need for string stream

AnimationDataDae::AnimationDataDae(const char* filePath) 
{
	readDaeFile(filePath);
}

void AnimationDataDae::readDaeFile(const char* filePath)
{
	std::stringstream textLineFromFile;

	endTime = 0;
	int count;
	XMFLOAT4 splitTransform;
	SkeletonAnimationData* newBone = new SkeletonAnimationData;
	std::string transOrRot;
	std::string textFromDaeFile;
	tinyxml2::XMLDocument daeFile;
	tinyxml2::XMLError loadFailed = daeFile.LoadFile(filePath);

	if (!loadFailed)
	{
		tinyxml2::XMLElement *parent = daeFile.FirstChildElement("COLLADA")->FirstChildElement("library_animations");
		for (tinyxml2::XMLElement *child = parent->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
		{
			bool isSameBone = false;
			textLineFromFile = std::stringstream(child->Attribute("id"));
			getline(textLineFromFile,textFromDaeFile, '.');
			if (newBone->boneName != textFromDaeFile)
			{
				newBone = new SkeletonAnimationData;
				isSameBone = true;
			}
			newBone->boneName = textFromDaeFile;
			getline(textLineFromFile, transOrRot, '.'); //lets us know if it's translation or toation may need to change though
			int loopCycle = 1;
			for (tinyxml2::XMLElement *floatArray = child->FirstChildElement(); floatArray != NULL; floatArray = floatArray->NextSiblingElement())
			{
				//we only want the input and output array, this is done in two loops if we look at the DAE file, we dont need the stuff after it.
				if (loopCycle == 1 || loopCycle == 2)
				{
					textFromDaeFile = std::string(floatArray->FirstChildElement()->Attribute("count"));
					count = std::stoi(textFromDaeFile);
					textLineFromFile = std::stringstream(floatArray->FirstChildElement("float_array")->GetText());
					if (transOrRot == "translate")
					{
						for (int i = 0; i < count; i++)
								{
									getline(textLineFromFile, textFromDaeFile, ' ');
									if (loopCycle == 1) //one means input which is time and two means output which is xyz co-ords
									{
										float time = std::stof(textFromDaeFile);
										newBone->tranTime.push_back(time);

										if (endTime < time)
											endTime = time;
									}
									else {
										splitTransform.x = std::stof(textFromDaeFile) * 0.1f;
										getline(textLineFromFile, textFromDaeFile, ' ');
										i++;
										splitTransform.y = std::stof(textFromDaeFile) * 0.1f;
										getline(textLineFromFile, textFromDaeFile, ' ');
										i++;
										splitTransform.z = std::stof(textFromDaeFile) * 0.1f;
										splitTransform.w = 0.0f;
										
										newBone->translate.push_back(splitTransform);	
									}
									
								}
					} 
					else if (transOrRot == "rotateX")
					{
						for (int i = 0; i < count; i++)
						{
							getline(textLineFromFile, textFromDaeFile, ' ');
							if (loopCycle == 1)
							{
								float time = std::stof(textFromDaeFile);
								if (endTime < time)
									endTime = time;
								newBone->rotTime.push_back(time);
							}
							else {
								XMFLOAT4 pushback = { std::stof(textFromDaeFile), 0.0f,0.0f,0.0f };
								newBone->rotations.push_back(pushback);
							//	newBone->rotX.push_back(std::stof(textFromDaeFile));

							}//may need to change newbone from a struct to a class

						}
					}
					else if (transOrRot == "rotateY")
					{
						for (int i = 0; i < count; i++)
						{
							getline(textLineFromFile, textFromDaeFile, ' ');
							if (loopCycle == 2) 
							{
								newBone->rotations[i].y = std::stof(textFromDaeFile);
								//newBone->rotY.push_back(std::stof(textFromDaeFile));
							}
						}
					}
					else if (transOrRot == "rotateZ")
					{
						for (int i = 0; i < count; i++)
						{
							getline(textLineFromFile, textFromDaeFile, ' ');
							if (loopCycle == 2)
							{
								newBone->rotations[i].z = std::stof(textFromDaeFile);
								//newBone->rotZ.push_back(std::stof(textFromDaeFile));
							}
						}
					}
					
				}
				else {
					break;
				}
				loopCycle++;
			}
			if (isSameBone)
			{
				boneAnimation.insert(std::make_pair(newBone->boneName, newBone));
			}
		}
	}
	else 
	{

	}

}

AnimationDataDae::~AnimationDataDae()
{
	std::for_each(boneAnimation.begin(), boneAnimation.end(), [](std::pair<std::string, SkeletonAnimationData* > data)
	{
		//key is the first value, second is the value of the animation data on the heap.
		delete data.second;
	});
}

SkeletonAnimationData::SkeletonAnimationData() : tranCurrentFrame(0), rotCurrentFrame(0)
{
	previousRotationTime = 0;
	previousTranslationTime = 0;
}
