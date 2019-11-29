#ifndef ROBOT_H
#define ROBOT_H

#include "AnimationDataDae.h"
#include "Application.h"
#include "CommonApp.h"
#include <iostream>
#include <String>
#include <fstream>
#include <vector>
#include "Skeleton.h"
#include <algorithm> // need for std::replace that erase uses
#include <sstream> // need for string stream
#include "Transform.h"

__declspec(align(16)) class Robot
{
public:
	Robot(std::string filePath, float wPosX, float wPosY, float wPosZ, float wPosW, float wRotX, float wRotY, float wRotZ, float wRotW);
	void ReadTextFileAndSetUpModel(std::string filepath);
	~Robot(void);

	void SetWorldPosition(float wposX, float wposY, float wposZ, float wposW, float wrotX, float wrotY, float wrotZ, float wrotW);
	void LoadResources(Robot* robotmesh);
	XMFLOAT4 GetWorldPosition();
	XMFLOAT4 GetRootPosition();
	void ChangeMeshToShadow(CommonApp::Shader& shader);
	void ReleaseResources(void);
	void UpdateMatrices(void);
	void Update();
	void DrawAll(void);
	void DrawShadow(void);
	void SetUpMeshes();

	void SetUpAnimations(void);
	
	
	//void SetWorldPosition(float fX, float fY, float fZ);
private:
	Skeleton* skeleton;
	std::vector<Skeleton> skeletonParts;
	//std::vector<std::string> skeletonPartsParent;
	//Instead of the string I've changed it to the position in the array, saves me looping through for loops.
	std::vector<int> skeletonPartsParent; 
	std::string folderName;
	XMFLOAT4 m_v4WorldPosition;
	XMFLOAT4 m_v4RotationPosition;
	std::vector<CommonMesh*> meshCollection;
	std::vector<CommonMesh*> shadowMeshCollection;
	XMMATRIX m_mWorldPosition;
	Transform transform;
	int count = 0;
	float animTime;

	AnimationDataDae* currentAnimation;

	AnimationDataDae* animationAttack;
	AnimationDataDae* animationIdle;
	AnimationDataDae* animationDeath;
};
#endif
