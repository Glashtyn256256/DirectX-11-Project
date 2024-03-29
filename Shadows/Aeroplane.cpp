//*********************************************************************************************
// File:			Aeroplane.cpp
// Description:		A very simple class to represent an aeroplane as one object with all the 
//					hierarchical components stored internally within the class.
// Module:			Real-Time 3D Techniques for Games
// Created:			Jake - 2010-2011
// Notes:			
//*********************************************************************************************

#include "Aeroplane.h"

AeroplaneMeshes *AeroplaneMeshes::Load()
{
	AeroplaneMeshes *pMeshes = new AeroplaneMeshes;

	pMeshes->pPlaneMesh = CommonMesh::LoadFromXFile(Application::s_pApp, "Resources/Plane/plane.x");
	pMeshes->pPropMesh = CommonMesh::LoadFromXFile(Application::s_pApp, "Resources/Plane/prop.x");
	pMeshes->pTurretMesh = CommonMesh::LoadFromXFile(Application::s_pApp, "Resources/Plane/turret.x");
	pMeshes->pGunMesh = CommonMesh::LoadFromXFile(Application::s_pApp, "Resources/Plane/gun.x");
	pMeshes->pBulletMesh = CommonMesh::LoadFromXFile(Application::s_pApp, "Resources/Plane/bullet.x");
	//pMeshes->pBombMesh = CommonMesh::LoadFromXFile(Application::s_pApp, "Resources/Plane/bomb.x");

	if (!pMeshes->pPlaneMesh || !pMeshes->pPropMesh || !pMeshes->pTurretMesh || !pMeshes->pGunMesh)
	{
		delete pMeshes;
		return NULL;
	}

	return pMeshes;
}

AeroplaneMeshes::AeroplaneMeshes() :
pPlaneMesh(NULL),
pPropMesh(NULL),
pTurretMesh(NULL),
pGunMesh(NULL),
pBulletMesh(NULL)
{
}

AeroplaneMeshes::~AeroplaneMeshes()
{
	delete this->pPlaneMesh;
	delete this->pPropMesh;
	delete this->pTurretMesh;
	delete this->pGunMesh;
	delete this->pBulletMesh;

}

Aeroplane::Aeroplane( float fX, float fY, float fZ, float fRotY )
{
	m_mWorldMatrix = XMMatrixIdentity();
	m_mPropWorldMatrix = XMMatrixIdentity();
	m_mTurretWorldMatrix = XMMatrixIdentity();
	m_mGunWorldMatrix = XMMatrixIdentity();
	m_mCamWorldMatrix = XMMatrixIdentity();

	m_v4Rot = XMFLOAT4(0.0f, fRotY, 0.0f, 0.0f);
	m_v4Pos = XMFLOAT4(fX, fY, fZ, 0.0f);

	m_v4PropOff = XMFLOAT4(0.0f, 0.0f, 1.9f, 0.0f);
	m_v4PropRot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	m_v4TurretOff = XMFLOAT4(0.0f, 1.05f, -1.3f, 0.0f);
	m_v4TurretRot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	m_v4GunOff = XMFLOAT4(0.0f, 0.5f, 0.0f, 0.0f);
	m_v4GunRot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	m_v4CamOff = XMFLOAT4(0.0f, 4.5f, -15.0f, 0.0f);
	m_v4CamRot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	m_vCamWorldPos = XMVectorZero();
	m_vForwardVector = XMVectorZero();

	m_fSpeed = 0.0f;

	m_bGunCam = false;

	normalMotionDeltaTime = 1.0 / 60.0f;

		
	forwardVectorOff = false;
}

Aeroplane::~Aeroplane( void )
{
}

void Aeroplane::SetWorldPosition( float fX, float fY, float fZ  )
{
	m_v4Pos = XMFLOAT4(fX, fY, fZ, 0.0f);
	UpdateMatrices();
}

Aeroplane::GunBullet::GunBullet(XMMATRIX bulletworldposition)
{
	XMMATRIX  mScale, mTran;

	bulletOffset = { 0.0f, 0.0f, 1.4f, 0.0f };
	bulletRotation = { 0.0f,0.0f,0.0f,0.0f };
	bulletScale = { 0.1f, 0.1f, 0.1f, 0.0f };
	mTran = XMMatrixTranslationFromVector(XMLoadFloat4(&bulletOffset));
	mScale = XMMatrixScalingFromVector(XMLoadFloat4(&bulletScale));
	bulletWorldPosition = mScale * mTran * bulletworldposition;
	survivalTime = 0.0f;
	speedBullet = 4.0f;
}

//Bomb::Bomb(XMMATRIX bombworldposition)
//{
//
//	XMMATRIX  mScale, mTran;
//	bombOffset = { 0.0f ,-2.0f ,0.0f ,0.0f };
//	//bombRotation = { 0.0f,0.0f,0.0f,0.0f };
//	bombScale = {0.8f,0.8f,0.8f,0.0 };
//	bombVelocity = { 0.1f,0.1f,0.0f,0.0f };
//	bombGravity = { 0.0f,-0.05f,0.0f,0.0f };
//	//mTran = XMMatrixTranslationFromVector(XMLoadFloat4(&bombOffset));
//	//mScale = XMMatrixScalingFromVector(XMLoadFloat4(&bombScale));
//	bombWorldPosition =  bombworldposition;
//}

//XMVECTOR Aeroplane::GetForwardVector() {
//	return m_vForwardVector * m_fSpeed;
//
//}
void Aeroplane::UpdateMatrices( void )
{
	XMMATRIX mRotX, mRotY, mRotZ, mTrans;
	XMMATRIX mPlaneCameraRot, mForwardMatrix;
	

	m_mWorldMatrix = XMMatrixIdentity();
	mRotX = XMMatrixRotationX(XMConvertToRadians(m_v4Rot.x));
	mRotY = XMMatrixRotationY(XMConvertToRadians(m_v4Rot.y));
	mRotZ = XMMatrixRotationZ(XMConvertToRadians(m_v4Rot.z));
	m_mWorldTrans = XMMatrixTranslation(m_v4Pos.x, m_v4Pos.y, m_v4Pos.z);

	m_mWorldMatrix = mRotZ * mRotX * mRotY * m_mWorldTrans;


	mPlaneCameraRot = mRotY * m_mWorldTrans;


	m_vForwardVector = m_mWorldMatrix.r[2];

	m_mPropWorldMatrix = XMMatrixIdentity();
	mRotX = XMMatrixRotationX(XMConvertToRadians(m_v4PropRot.x));
	mRotY = XMMatrixRotationY(XMConvertToRadians(m_v4PropRot.y));
	mRotZ = XMMatrixRotationZ(XMConvertToRadians(m_v4PropRot.z));
	mTrans = XMMatrixTranslation(m_v4PropOff.x, m_v4PropOff.y, m_v4PropOff.z);

	m_mPropWorldMatrix =  mRotX * mRotY * mRotZ * mTrans * m_mWorldMatrix;


	m_mTurretWorldMatrix = XMMatrixIdentity();
	mRotX = XMMatrixRotationX(XMConvertToRadians(m_v4TurretRot.x));
	mRotY = XMMatrixRotationY(XMConvertToRadians(m_v4TurretRot.y));
	mRotZ = XMMatrixRotationZ(XMConvertToRadians(m_v4TurretRot.z));
	mTrans = XMMatrixTranslation(m_v4TurretOff.x, m_v4TurretOff.y, m_v4TurretOff.z);

	m_mTurretWorldMatrix =  mRotX * mRotY * mRotZ * mTrans * m_mWorldMatrix;


	m_mGunWorldMatrix = XMMatrixIdentity();
	mRotX = XMMatrixRotationX(XMConvertToRadians(m_v4GunRot.x));
	mRotY = XMMatrixRotationY(XMConvertToRadians(m_v4GunRot.y));
	mRotZ = XMMatrixRotationZ(XMConvertToRadians(m_v4GunRot.z));
	mTrans = XMMatrixTranslation(m_v4GunOff.x, m_v4GunOff.y, m_v4GunOff.z);

	m_mGunWorldMatrix =  mRotX * mRotY * mRotZ * mTrans * m_mTurretWorldMatrix;


	m_mCamWorldMatrix = XMMatrixIdentity();
	mRotX = XMMatrixRotationX(XMConvertToRadians(m_v4CamRot.x));
	mRotY = XMMatrixRotationY(XMConvertToRadians(m_v4CamRot.y));
	mRotZ = XMMatrixRotationZ(XMConvertToRadians(m_v4CamRot.z));
	mTrans = XMMatrixTranslation(m_v4CamOff.x, m_v4CamOff.y, m_v4CamOff.z);


	if( m_bGunCam )
		m_mCamWorldMatrix =  mTrans * mRotX * mRotY * mRotZ * m_mGunWorldMatrix;
	else
		m_mCamWorldMatrix =  mTrans * mRotX * mRotY * mRotZ * mPlaneCameraRot;
	
	// Get the camera's world position out of the matrix
	m_vCamWorldPos = m_mCamWorldMatrix.r[3];

	if (bulletContainer.size() > 0)
	{
		for (int i = 0; i < bulletContainer.size(); i++)
		{

			mTrans = XMMatrixTranslationFromVector(XMLoadFloat4(&bulletContainer[i]->bulletOffset));
			bulletContainer[i]->bulletWorldPosition = mTrans * bulletContainer[i]->bulletWorldPosition;
		}
	}
	//if (bombDropped)
	//{
	//	mTrans = XMMatrixTranslationFromVector(XMLoadFloat4(&newBomb->GetBombOffset()));
	//	XMMATRIX bombWorldMatrix = mTrans * newBomb->GetBombWorldMatrix();
	//	newBomb->SetBombWorldMatrix(bombWorldMatrix);
	//}
}

void Aeroplane::deleteBullet()
{
	for (int i = 0; i < bulletContainer.size(); i++)
	{
		if (bulletContainer[i]->survivalTime > 5.0f)
		{

			delete(bulletContainer[i]); //delete the pointer stop memory leaks
			bulletContainer.erase(bulletContainer.begin()); //remove the empty vector position
			--i; //Go back to the start of the vector
		}

	}
}

void Aeroplane::Update( bool bPlayerControl )
{
	if( bPlayerControl )
	{

		if (Application::s_pApp->IsKeyPressed('Q') && m_v4Rot.x < 60)
			m_v4Rot.x += 2.0f;
		else if (!Application::s_pApp->IsKeyPressed('Q') && m_v4Rot.x > 0)
			m_v4Rot.x -= 1.0f;

		if (Application::s_pApp->IsKeyPressed('A') && m_v4Rot.x > -60 )
			m_v4Rot.x -= 2.0f;
		else if (!Application::s_pApp->IsKeyPressed('A') && m_v4Rot.x < 0)
			m_v4Rot.x += 1.0f;


		if( Application::s_pApp->IsKeyPressed('O') )
		{
			m_v4Rot.y -= 1.0f;
			if (m_v4Rot.z < 20)
				m_v4Rot.z += 2.0f;
		}
		else if (m_v4Rot.z > 0)
			m_v4Rot.z -= 1.0f;

		if( Application::s_pApp->IsKeyPressed('P') )
		{
			m_v4Rot.y += 1.0f;
			if (m_v4Rot.z > -20)
				m_v4Rot.z -= 2.0f;
		}
		else if (m_v4Rot.z < 0)
			m_v4Rot.z += 1.0f;

	} // End of if player control

	if (Application::s_pApp->IsKeyPressed(VK_SPACE))
	{
		//instantiate a new bullet
		newBullet = new GunBullet(m_mGunWorldMatrix);
		//add it to the vector of bullets
		bulletContainer.push_back(newBullet);
	}

	
	static bool dbM = false;
	if (Application::s_pApp->IsKeyPressed('M'))
	{
		if (dbM == false)
		{
			if (!forwardVectorOff)
			{
				forwardVectorOff = true;
			}
			else {
				forwardVectorOff = false;
			}
			dbM = true;
		}
	}
	else
	{
		dbM = false;
	}

	// Apply a forward thrust and limit to a maximum speed of 1
	m_fSpeed += 0.001f;

	if (m_fSpeed > 1)
		m_fSpeed = 1;
	
	if (bulletContainer.size() > 0)
	{
		deleteBullet();
		for (int i = 0; i < bulletContainer.size(); i++)
		{
			bulletContainer[i]->survivalTime += normalMotionDeltaTime;
			bulletContainer[i]->bulletOffset.z += bulletContainer[i]->speedBullet;// + m_fSpeed;
		}
	}
	
	// Rotate propellor and turret
	m_v4PropRot.z += 100 * m_fSpeed;
	
	UpdateMatrices();

	// Move Forward
	if (!forwardVectorOff)
	{
		XMVECTOR vCurrPos = XMLoadFloat4(&m_v4Pos);
		vCurrPos += m_vForwardVector * m_fSpeed;
		XMStoreFloat4(&m_v4Pos, vCurrPos);
	}
}




void Aeroplane::Draw(const AeroplaneMeshes *pMeshes)
{
	Application::s_pApp->SetWorldMatrix(m_mWorldMatrix);
	pMeshes->pPlaneMesh->Draw();

	Application::s_pApp->SetWorldMatrix(m_mPropWorldMatrix);
	pMeshes->pPropMesh->Draw();

	Application::s_pApp->SetWorldMatrix(m_mTurretWorldMatrix);
	pMeshes->pTurretMesh->Draw();

	Application::s_pApp->SetWorldMatrix(m_mGunWorldMatrix);
	pMeshes->pGunMesh->Draw();

	for (int i = 0; i < bulletContainer.size(); i++)
	{
		Application::s_pApp->SetWorldMatrix(bulletContainer[i]->bulletWorldPosition);
		pMeshes->pBulletMesh->Draw();
	}
}



