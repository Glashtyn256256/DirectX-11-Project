#include "Robot.h"


Robot::Robot(std::string filepath, XMFLOAT4 worldpos, XMFLOAT4 worldrot, XMFLOAT4 camerapos, XMFLOAT4 camerarot)
{
	SetWorldPosition(worldpos, worldrot);
	SetCameraPosition(camerapos, camerarot);
	ReadTextFileAndSetUpModel(filepath);
	SetUpMeshes();
	SetUpAnimations();

	

}

void Robot::ReadTextFileAndSetUpModel(std::string filepath)
{
	std::string textLineFromFile;
	std::fstream textFile(filepath);

	std::getline(textFile, textLineFromFile);
	textLineFromFile.erase(std::remove(textLineFromFile.begin(),
		textLineFromFile.end(), '\"'),
		textLineFromFile.end());
	folderName = textLineFromFile;

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
	animationAttack = new AnimationDataDae("Resources/Robot/MayaFiles/RobotAttackAnim.dae");
	animationIdle = new AnimationDataDae("Resources/Robot/MayaFiles/RobotIdleAnim.dae");
	animationDeath = new AnimationDataDae("Resources/Robot/MayaFiles/RobotDieAnim.dae");

	currentAnimation = nullptr;
	animTime = 0;
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

void Robot::Update(float time)
{
	if (Application::s_pApp->IsKeyPressed('D'))
		m_v4CamOff.z -= 2.0f;

	if (Application::s_pApp->IsKeyPressed('G'))
		m_v4CamOff.z += 2.0f;

	if (Application::s_pApp->IsKeyPressed('R'))
		m_v4CamOff.y -= 2.0f;

	if (Application::s_pApp->IsKeyPressed('F'))
		m_v4CamOff.y += 2.0f;

	if (Application::s_pApp->IsKeyPressed('T'))
		m_v4CamOff.x -= 2.0f;

	if (Application::s_pApp->IsKeyPressed('E'))
		m_v4CamOff.x += 2.0f;


	if (Application::s_pApp->IsKeyPressed('1'))
	{
	//	previousAnimation = currentAnimation;
		currentAnimation = animationAttack;
	}
	if (Application::s_pApp->IsKeyPressed('2'))
	{
	//	previousAnimation = currentAnimation;
		currentAnimation = animationIdle;
	}
	if (Application::s_pApp->IsKeyPressed('3'))
	{
	//	previousAnimation = currentAnimation;
		currentAnimation = animationDeath;
	}

	if (currentAnimation)
	{

		//debug helper delete when done
		if (count == 86) {
			count++;
		}

		//deltatime
		
		for (int i = 0; i < skeletonParts.size(); i++)
		{

			//Gets the correct animation thanks to the naming
			SkeletonAnimationData* data = currentAnimation->boneAnimation[skeletonParts[i]->GetPartName()];
			if (data)
			{
				Skeleton* bone = skeletonParts[i];

				if (animTime >= currentAnimation->endTime)
				{
					data->rotCurrentFrame = 0;
					data->tranCurrentFrame = 0;
				}
					int test(data->tranTime.size() - 1);
					int currentTranFrame = data->tranCurrentFrame;
					if (!(currentTranFrame > test))
					{
						float translationEndTime = data->tranTime[currentTranFrame];
						XMFLOAT4 previousTranslation;
						XMFLOAT4 currentTranslation;

						if (currentTranFrame == 0)
						{
							previousTranslation = bone->GetOffsetPosition();
						}
						else
						{
							previousTranslation = data->translate[currentTranFrame - 1];
						}

						if (!(animTime >= translationEndTime))
						{
							float tLerp = (animTime - data->previousTranslationTime) / (translationEndTime - data->previousTranslationTime);
							currentTranslation = data->translate[currentTranFrame];
							bone->SetSkeletonOffsetPosition(ReturnLerpedPosition(previousTranslation, currentTranslation, tLerp));
						}
						else
						{
							count++;
						}
						if (animTime >= translationEndTime)
						{
							data->previousTranslationTime = translationEndTime;

							currentTranFrame++;
							if (currentTranFrame > data->tranTime.size())
								currentTranFrame = data->tranTime.size();
						}
						data->tranCurrentFrame = currentTranFrame;
					}

					int test1(data->rotTime.size() - 1);
					int currentRotFrame = data->rotCurrentFrame;
					if (!(currentRotFrame > test1))
					{

						float rotationEndTime = data->rotTime[currentRotFrame];
						XMFLOAT4 previousRotation;
						XMFLOAT4 currentRotation;

						if (currentRotFrame == 0)
						{
							previousRotation = bone->GetRotationPosition();
						}
						else
						{
							previousRotation = data->rotations[currentRotFrame - 1];
						}

						if (!(animTime >= rotationEndTime))
						{
							float rLerp = (animTime - data->previousRotationTime) / (rotationEndTime - data->previousRotationTime);
							currentRotation = data->rotations[currentRotFrame];
							bone->SetSkeletonRotationPosition(ReturnLerpedPosition(previousRotation, currentRotation, rLerp));

						//Animation 1 we lerp returnLerpedPosition(current anim - >previousRotation,currentanim -> currentRotation, rLerp)
						//animation2 2 returnLerpedPosition(changed animation ->previousRotation, changed anim -> currentRotation, rLerp)
						//bone->SetSkeletonRotationPosition(ReturnLerpedPosition(animtion1, animtion2, time)}
						}

						if (animTime >= rotationEndTime)
						{
							data->previousRotationTime = rotationEndTime;

							currentRotFrame++;
							if (currentRotFrame > data->rotTime.size())
								currentRotFrame = data->rotTime.size();
						}
						data->rotCurrentFrame = currentRotFrame;
					}
			}
		}


		if (animTime >= currentAnimation->endTime)
		{
			animTime = 0;
			currentAnimation = nullptr;
		}
		count++;
		animTime += time;
	}
	UpdateMatrices();
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
			foldername = "Resources/" + folderName + "/" + skeletonParts[i]->GetPartName() + ".x";
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

