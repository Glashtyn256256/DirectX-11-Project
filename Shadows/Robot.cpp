#include "Robot.h"


Robot::Robot(std::string filepath, XMFLOAT4 worldpos, XMFLOAT4 worldrot, XMFLOAT4 camerapos, XMFLOAT4 camerarot)
{
	SetWorldPosition(worldpos, worldrot);
	SetCameraPosition(camerapos, camerarot);
	ReadTextFileAndSetUpModel(filepath);
	SetUpMeshes();
	SetUpAnimations();
	hasRobotBeenHit = false;
	RobotAttackPlane = false;
	

}

void Robot::ReadTextFileAndSetUpModel(std::string filepath)
{
	std::string textLineFromFile;
	std::fstream textFile(filepath);

	//std::getline(textFile, textLineFromFile);
	//textLineFromFile.erase(std::remove(textLineFromFile.begin(),
	//	textLineFromFile.end(), '\"'),
	//	textLineFromFile.end());
	//folderName = textLineFromFile;

	while (!textFile.eof())
	{
		skeleton = new Skeleton();
		std::getline(textFile, textLineFromFile);
		//textLineFromFile.erase(std::find(textLineFromFile.begin(), textLineFromFile.end(),'\"'));
		//textLineFromFile.replace(textLineFromFile.begin(), textLineFromFile.end(), "\"", "");
		textLineFromFile.erase(std::remove(textLineFromFile.begin(),
			textLineFromFile.end(), '\"'),
			textLineFromFile.end());
		skeleton->SetPartName(textLineFromFile);

		std::getline(textFile, textLineFromFile);
		textLineFromFile.erase(std::remove(textLineFromFile.begin(),
			textLineFromFile.end(), '\"'),
			textLineFromFile.end());
		skeleton->SetParentName(textLineFromFile);

		getline(textFile, textLineFromFile);
		std::vector<float> splitFloat;
		std::stringstream splitString(textLineFromFile);

		for (size_t i = 0; i < 3; i++)
		{
			getline(splitString, textLineFromFile, ',');
			splitFloat.push_back(std::stof(textLineFromFile));
		}
		skeleton->SetSkeletonOffsetPosition(splitFloat[0] / 10, splitFloat[1] / 10, splitFloat[2] / 10, 0.f);
		splitFloat.clear();

		skeleton->SetSkeletonRotationPosition(0.0f, 0.0f, 0.0f, 0.0f);

		skeleton->SetLocalMatrix(); //now we have pos and rot we can set up the local matrix for the bone;
		skeletonParts.push_back(skeleton);
		skeletonParentBone.insert(std::make_pair(skeleton->GetPartName(), skeleton));

	}
}

void Robot::SetUpMeshes()
{
	for (int i = 1; i < skeletonParts.size(); i++) //Start at one because you don't need a mesh for the root
	{
		meshCollection.push_back(nullptr);
		shadowMeshCollection.push_back(nullptr);
	}
};

void Robot::SetUpAnimations(void) {
	animationAttack = new AnimationDataDae("Resources/Robot/Anims/RobotAttackAnim.dae");
	animationIdle = new AnimationDataDae("Resources/Robot/Anims/RobotIdleAnim.dae");
	animationDeath = new AnimationDataDae("Resources/Robot/Anims/RobotDieAnim.dae");

	currentAnimation = nullptr;
	animationElapsedTime = 0;
}

void Robot::SetCameraPosition(XMFLOAT4 camerapos, XMFLOAT4 camerarot)
{
	m_v4CamOff = camerapos;
	m_v4CamRot = camerarot;
	m_vCamWorldPos = XMVectorZero();
	m_mCamWorldMatrix = XMMatrixIdentity();
}

Robot::~Robot(void)
{
	delete animationAttack;
	delete animationIdle;
	delete animationDeath;

	std::for_each(skeletonParentBone.begin(), skeletonParentBone.end(), [](std::pair<std::string, Skeleton* > skeletonBone)
	{
		//key is the first value, second are the pointers we delete.
		delete skeletonBone.second;
	});
}

void Robot::UpdateMatrices(void)
{
	for (int i = 0; i < skeletonParts.size(); i++)
	{
		skeletonParts[i]->SetLocalMatrix();
		//for this to work needs root in the txt file, could change to 0 so if i == 0
		if (skeletonParts[i]->GetPartName() == "root")
		{
			skeletonParts[i]->SetWorldMatrix(m_mWorldPosition);
		}
		else
		{
			skeleton = skeletonParentBone[skeletonParts[i]->GetParentName()];
			skeletonParts[i]->SetWorldMatrix(skeleton->GetWorldMatrix());
		}
	}

	m_mCamWorldMatrix = transform.CalculateWorldMatrix(transform.CalculateLocalMatrix(m_v4CamOff, m_v4CamRot), m_mWorldPosition);
	m_vCamWorldPos = m_mCamWorldMatrix.r[3];
}

void Robot::Update(float time, Aeroplane* plane)
{

	if (Application::s_pApp->IsKeyPressed('1'))
	{
		/*if (currentAnimation != animationIdle)
		{
			previousAnimation = currentAnimation;
		}*/
		if (currentAnimation == nullptr)
		{
			currentAnimation = animationIdle;
		}
		//animationElapsedTime = 0;
	}

	if (Application::s_pApp->IsKeyPressed('2'))
	{
		/*if (currentAnimation != animationAttack)
		{
			previousAnimation = currentAnimation;
		}*/
		if (currentAnimation == nullptr)
		{
			currentAnimation = animationAttack;
		}
		
		//animationElapsedTime = 0;
	}

	if (Application::s_pApp->IsKeyPressed('3'))
	{
		/*if (currentAnimation != animationDeath)
		{
			previousAnimation = currentAnimation;
		}*/
		if (currentAnimation == nullptr)
		{
			currentAnimation = animationDeath;
		}
		//animationElapsedTime = 0;
	}

	if (PlaneAttackWhenClose(plane))
	{
		if (currentAnimation == nullptr)
		{
			currentAnimation = animationAttack;
		}
		
	}

	if (!hasRobotBeenHit)
	{
		
		//since all the robots point to the same animations they all die when only one should get hit xD
		//Also hitbybullets is not working correctly not got the time to finish it.
		/*if (HitByBullet(plane))
		{
			currentAnimation = animationDeath;
			hasRobotBeenHit = true;
		}*/
	}
	//if (currentAnimation && previousAnimation)
	//{
	//	//PlayAnimation(currentAnimation,previousAnimation, time);
	//}
	//else 
	
	if (currentAnimation)
	{
		PlayAnimation(currentAnimation, time);
	}

	UpdateMatrices();
}

bool Robot::PlaneAttackWhenClose(Aeroplane* plane)
{
	float distance = 0.0f;
	float xtemp = 0.0f, ytemp = 0.0f, ztemp = 0.0f;

	for (int b = 0; b < skeletonParts.size(); b++)
	{
		xtemp = skeletonParts[b]->GetOffsetXPosition() - plane->GetXPosition();
		ytemp = skeletonParts[b]->GetOffsetYPosition() - plane->GetYPosition();
		ztemp = skeletonParts[b]->GetOffsetZPosition() - plane->GetZPosition();
		distance = sqrt(pow(xtemp, 2) + pow(ytemp, 2) + pow(ztemp, 2));

		if (distance < 40.0f)
		{
			return true;

		}
	}
	return false;
}
bool Robot::HitByBullet(Aeroplane* plane)
{
	float distance = 0.0f;
	float xtemp = 0.0f, ytemp = 0.0f, ztemp = 0.0f;
	
		for (int i = 0; i < plane->bulletContainer.size(); i++)
		{
			for (int b = 0; b < skeletonParts.size(); b++)
			{
				xtemp = skeletonParts[b]->GetOffsetXPosition() - plane->bulletContainer[i]->bulletOffset.x;
				ytemp = skeletonParts[b]->GetOffsetYPosition() - plane->bulletContainer[i]->bulletOffset.y;
				ztemp = skeletonParts[b]->GetOffsetZPosition() - plane->bulletContainer[i]->bulletOffset.z;

				distance = sqrt(pow(xtemp, 2) + pow(ytemp, 2) + pow(ztemp, 2));

				if (distance < 5.0f)
				{
					return true;
				}
			}
			  

		}
	return false;
}
void Robot::PlayAnimation(AnimationDataDae*& currentAnimation, float time)
{
	for (int i = 0; i < skeletonParts.size(); i++)
	{

		//Gets the correct animation thanks to the naming
		SkeletonAnimationData* data = currentAnimation->boneAnimation[skeletonParts[i]->GetPartName()];
		if (data)
		{
			Skeleton* bone = skeletonParts[i];

			int translationTimeArraySize(data->translationTimes.size() - 1);

			int currentTransformationFrame = data->translationCurrentFrame;
			if (!(currentTransformationFrame > translationTimeArraySize))
			{
				float translationEndTime = data->translationTimes[currentTransformationFrame];
				XMFLOAT4 previousTranslation;
				XMFLOAT4 currentTranslation;

				if (currentTransformationFrame == 0)
				{
					//previousTranslation = bone->GetOffsetPosition();
					previousTranslation = data->translation[currentTransformationFrame];
				}
				else
				{
					previousTranslation = data->translation[currentTransformationFrame - 1];
				}

				if (!(animationElapsedTime >= translationEndTime))
				{
					float tLerp = (animationElapsedTime - data->previousTranslationTime) / (translationEndTime - data->previousTranslationTime);
					currentTranslation = data->translation[currentTransformationFrame];
					bone->SetSkeletonOffsetPosition(ReturnLerpedPosition(previousTranslation, currentTranslation, tLerp));
				}

				if (animationElapsedTime >= translationEndTime)
				{
					data->previousTranslationTime = translationEndTime;
					currentTransformationFrame++;
				}
			data->translationCurrentFrame = currentTransformationFrame;

			}

			int rotationTimeArraySize(data->rotationTimes.size() - 1);
			int currentRotationFrame = data->rotationCurrentFrame;
			if (!(currentRotationFrame > rotationTimeArraySize))
			{

					float rotationEndTime = data->rotationTimes[currentRotationFrame];
					XMFLOAT4 previousRotation;
					XMFLOAT4 currentRotation;

					if (currentRotationFrame == 0)
					{
						previousRotation = bone->GetRotationPosition();
					}
					else
					{
						previousRotation = data->rotations[currentRotationFrame - 1];
					}

					if (!(animationElapsedTime >= rotationEndTime))
					{
						float rLerp = (animationElapsedTime - data->previousRotationTime) / (rotationEndTime - data->previousRotationTime);
						currentRotation = data->rotations[currentRotationFrame];
						bone->SetSkeletonRotationPosition(ReturnLerpedPosition(previousRotation, currentRotation, rLerp));

						//Animation 1 we lerp returnLerpedPosition(current anim - >previousRotation,currentanim -> currentRotation, rLerp)
						//animation2 2 returnLerpedPosition(changed animation ->previousRotation, changed anim -> currentRotation, rLerp)
						//bone->SetSkeletonRotationPosition(ReturnLerpedPosition(animtion1, animtion2, time)}
					}

					if (animationElapsedTime >= rotationEndTime)
					{
						data->previousRotationTime = rotationEndTime;

						currentRotationFrame++;
					}
					data->rotationCurrentFrame = currentRotationFrame;

					/*if (animationElapsedTime >= currentAnimation->finalFrameEndTime)
					{
						data->ResetCurrentFrames();
						data->ResetPreviousTimes();

					}*/
			}
			if (animationElapsedTime >= currentAnimation->finalFrameEndTime)
			{
				data->ResetCurrentFrames();
				data->ResetPreviousTimes();

			}
		}

	}

	if (animationElapsedTime >= currentAnimation->finalFrameEndTime)
	{
		//data->ResetCurrentFrames();
		animationElapsedTime = 0;
		currentAnimation = nullptr;
	}
	count++;
	animationElapsedTime += time;
}

void Robot::PlayAnimation(AnimationDataDae*& currentAnimation, AnimationDataDae*& previousAnimation, float time)
{
	//for (int i = 0; i < skeletonParts.size(); i++)
	//{

	//	SkeletonAnimationData* previousData = previousAnimation->boneAnimation[skeletonParts[i]->GetPartName()];
	//	//Gets the correct animation thanks to the naming
	//	SkeletonAnimationData* data = currentAnimation->boneAnimation[skeletonParts[i]->GetPartName()];
	//	if (data)
	//	{
	//		Skeleton* bone = skeletonParts[i];

	//		if (animTime >= currentAnimation->endTime)
	//		{
	//			data->ResetCurrentFrames();
	//		}
	//		int test(data->tranTime.size() - 1);
	//		int currentTranFrame = data->tranCurrentFrame;
	//		int oldCurrentTranFrame = previousData->tranCurrentFrame;
	//		if (!(currentTranFrame > test))
	//		{
	//			float translationEndTime = data->tranTime[currentTranFrame];
	//			float oldTranslationEndTime = previousData->tranTime[oldCurrentTranFrame];
	//			XMFLOAT4 previousTranslation, currentTranslation, currentOffset;
	//			XMFLOAT4 oldCurrentTranslation, oldPreviousTranslation, oldOffset;
	//			float tLerp, oldtLerp;
	//			if (currentTranFrame == 0)
	//			{
	//				previousTranslation = bone->GetOffsetPosition();
	//				
	//			}
	//			else
	//			{
	//				previousTranslation = data->translate[currentTranFrame - 1];
	//			}
	//			if (oldCurrentTranFrame == 0)
	//			{
	//				oldPreviousTranslation = bone->GetOffsetPosition();
	//			}
	//			else
	//			{
	//				oldPreviousTranslation = previousData->translate[oldCurrentTranFrame - 1];
	//			}

	//			if (!(animTime >= translationEndTime))
	//			{
	//				float tLerp = (animTime - data->previousTranslationTime) / (translationEndTime - data->previousTranslationTime);										
	//				currentTranslation = data->translate[currentTranFrame];
	//				currentOffset = ReturnLerpedPosition(previousTranslation, currentTranslation, tLerp);
	//				if (currentTranFrame == 0)
	//				{
	//					oldtLerp = (animTime - previousData->previousTranslationTime) / (oldTranslationEndTime - previousData->previousTranslationTime);
	//					//oldtLerp = (animTime - previousData->previousTranslationTime) / (translationEndTime - previousData->previousTranslationTime);
	//					oldCurrentTranslation = previousData->translate[oldCurrentTranFrame];
	//					oldOffset = ReturnLerpedPosition(oldPreviousTranslation, oldCurrentTranslation, oldtLerp);
	//					bone->SetSkeletonOffsetPosition(ReturnLerpedPosition(oldOffset, currentOffset, time));
	//				}
	//				else
	//				{
	//					bone->SetSkeletonOffsetPosition(ReturnLerpedPosition(previousTranslation, currentTranslation, tLerp));
	//				}
	//				
	//			}
	//			else
	//			{
	//				count++;
	//			}
	//			if (animTime >= translationEndTime)
	//			{
	//				data->previousTranslationTime = translationEndTime;

	//				currentTranFrame++;
	//				if (currentTranFrame > data->tranTime.size())
	//					currentTranFrame = data->tranTime.size();
	//			}
	//			data->tranCurrentFrame = currentTranFrame;
	//		}

	//		int test1(data->rotTime.size() - 1);
	//		int currentRotFrame = data->rotCurrentFrame;
	//		int oldCurrentRotFrame = previousData->rotCurrentFrame;
	//		if (!(currentRotFrame > test1))
	//		{

	//			float rotationEndTime = data->rotTime[currentRotFrame];
	//			float oldRotationEndTime = data->rotTime[currentRotFrame];
	//			XMFLOAT4 previousRotation, currentRotation, currentRotationPosition;
	//			XMFLOAT4 oldPreviousRotation, oldCurrentRotation, oldCurrentRotationPosition;
	//			float rLerp, oldrLerp;

	//			if (currentRotFrame == 0)
	//			{
	//				previousRotation = bone->GetRotationPosition();
	//			}
	//			else
	//			{
	//				previousRotation = data->rotations[currentRotFrame - 1];
	//			}

	//			if (oldCurrentRotFrame == 0)
	//			{
	//				previousRotation = bone->GetRotationPosition();
	//			}
	//			else
	//			{
	//				oldPreviousRotation = data->rotations[oldCurrentRotFrame - 1];
	//			}
	//			if (!(animTime >= rotationEndTime))
	//			{
	//				rLerp = (animTime - data->previousRotationTime) / (rotationEndTime - data->previousRotationTime);
	//				currentRotation = data->rotations[currentRotFrame];
	//				
	//				currentRotationPosition = ReturnLerpedPosition(previousRotation, currentRotation, rLerp);
	//				if (currentRotFrame == 0)
	//				{
	//					oldrLerp = (animTime - previousData->previousRotationTime) / (rotationEndTime - previousData->previousRotationTime);
	//					oldCurrentRotation = previousData->rotations[oldCurrentRotFrame];
	//					oldCurrentRotationPosition = ReturnLerpedPosition(previousRotation, currentRotation, oldrLerp);
	//					bone->SetSkeletonRotationPosition(ReturnLerpedPosition(oldCurrentRotation, currentRotation, time));
	//				}
	//				else
	//				{
	//					bone->SetSkeletonRotationPosition(ReturnLerpedPosition(previousRotation, currentRotation, rLerp));
	//				}
	//				//Animation 1 we lerp returnLerpedPosition(current anim - >previousRotation,currentanim -> currentRotation, rLerp)
	//				//animation2 2 returnLerpedPosition(changed animation ->previousRotation, changed anim -> currentRotation, rLerp)
	//				//bone->SetSkeletonRotationPosition(ReturnLerpedPosition(animtion1, animtion2, time)}
	//				//We give the new animation the oldframecount
	//			}

	//			if (animTime >= rotationEndTime)
	//			{
	//				data->previousRotationTime = rotationEndTime;

	//				currentRotFrame++;
	//				if (currentRotFrame > data->rotTime.size())
	//					currentRotFrame = data->rotTime.size();
	//			}
	//			data->rotCurrentFrame = currentRotFrame;

	//			
	//		}
	//	}
	//}

	//if (animTime >= currentAnimation->endTime)
	//{
	//	animTime = 0;
	//	currentAnimation = nullptr;
	//	previousAnimation = nullptr;
	//}
	//count++;
	//animTime += time;

}

XMFLOAT4 Robot::ReturnLerpedPosition(XMFLOAT4 previous, XMFLOAT4 current, float lerptime)
{
	XMVECTOR previousPosition = XMLoadFloat4(&previous);
	XMVECTOR currentPosition = XMLoadFloat4(&current);
	
	XMVECTOR newPosition = XMVectorLerp(previousPosition, currentPosition, lerptime);
	XMFLOAT4 m_v4newPosition;
	XMStoreFloat4(&m_v4newPosition, newPosition);
	return m_v4newPosition;
}
void Robot::LoadResources(Robot* robotmesh)
{
	//mesh has root. reason meshcollection is i-1 is due to
	if (robotmesh->meshCollection[0] == NULL)
	{
		for (int i = 1; i < skeletonParts.size(); i++)
		{
			std::string foldername;
			//foldername = "Resources/" + folderName + "/" + skeletonParts[i]->GetPartName() + ".x";
			foldername = "Resources/Robot/" + skeletonParts[i]->GetPartName() + ".x";
			meshCollection[i - 1] = CommonMesh::LoadFromXFile(Application::s_pApp, foldername.c_str());
			//shadowMeshCollection[i - 1] = meshCollection[i - 1];
			shadowMeshCollection[i - 1] = CommonMesh::LoadFromXFile(Application::s_pApp, foldername.c_str());
			//^^this is needed since we need to start mesh collection from 0
		}
	}
	else
	{
		for (int i = 1; i < skeletonParts.size(); i++)
		{
			meshCollection[i - 1] = robotmesh->meshCollection[i - 1];
			shadowMeshCollection[i - 1] = robotmesh->shadowMeshCollection[i - 1];
		}
	}

}

XMFLOAT4 Robot::GetWorldPosition() {
	return m_v4WorldPosition;
}

//XMFLOAT4 Robot::GetRootPosition() {
//	XMVECTOR test1, test2;
//	test1 = XMLoadFloat4(&skeletonParts[0].GetOffsetPosition());
//	test2 = XMLoadFloat4(&m_v4WorldPosition);
//	test2 = XMVectorAdd(test1, test2);
//	XMFLOAT4 returnValue;
//	XMStoreFloat4(&returnValue, test2);
//	return returnValue;
//}

void Robot::ChangeMeshToShadow(CommonApp::Shader& shader)
{
	for (int i = 0; i < shadowMeshCollection.size(); i++)
	{
		shadowMeshCollection[i]->SetShaderForAllSubsets(&shader);
	}
}

void Robot::ReleaseResources(void)
{
	for (int i = 0; i < meshCollection.size(); i++)
	{
		delete meshCollection[i];
		delete shadowMeshCollection[i];
	}
}

void Robot::DrawAll(void)
{

	for (int i = 1; i < skeletonParts.size(); i++)
	{
		Application::s_pApp->SetWorldMatrix(skeletonParts[i]->GetWorldMatrix());
		meshCollection[i - 1]->Draw();
	}

}

void Robot::DrawShadow(void)
{

	for (int i = 1; i < skeletonParts.size(); i++)
	{
		Application::s_pApp->SetWorldMatrix(skeletonParts[i]->GetWorldMatrix());
		shadowMeshCollection[i - 1]->Draw();
	}

}

void Robot::SetWorldPosition(float wposX, float wposY, float wposZ, float wposW, float wrotX, float wrotY, float wrotZ, float wrotW) {
	m_v4WorldPosition.x = wposX;
	m_v4WorldPosition.y = wposY;
	m_v4WorldPosition.z = wposZ;
	m_v4WorldPosition.w = wposW;

	m_v4RotationPosition.x = wrotX;
	m_v4RotationPosition.y = wrotY;
	m_v4RotationPosition.z = wrotZ;
	m_v4RotationPosition.w = wrotW;

	m_mWorldPosition = transform.CalculateLocalMatrix(m_v4WorldPosition, m_v4RotationPosition);
}
void Robot::SetWorldPosition(XMFLOAT4 worldpos, XMFLOAT4 worldrot)
{
	m_v4WorldPosition = worldpos;
	m_v4RotationPosition = worldrot;

	m_mWorldPosition = transform.CalculateLocalMatrix(m_v4WorldPosition, m_v4RotationPosition);
}
;

