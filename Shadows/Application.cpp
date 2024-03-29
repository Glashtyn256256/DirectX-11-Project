#include "Application.h"
#include "HeightMap.h"
#include "Aeroplane.h"
#include "Robot.h"


Application* Application::s_pApp = NULL;


const int CAMERA_MAP = 0;
const int CAMERA_PLANE = 1;
const int CAMERA_GUN = 2;
const int CAMERA_ROBOT = 3;
const int CAMERA_BOMB = 4;
const int CAMERA_MAX = 5;

//static const int RENDER_TARGET_WIDTH = 512;
//static const int RENDER_TARGET_HEIGHT = 512;

static const int RENDER_TARGET_WIDTH = 16350;
static const int RENDER_TARGET_HEIGHT = 16350;

static const float AEROPLANE_RADIUS = 6.0f;
static const float ROBOT_RADIUS = 620.f;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool Application::HandleStart()
{
	//Instead of divding better pefroamnce if we calculate it 1.0f / 600 slowmo and 1.0f / 60 for rnoaml;
	slowMotionTime = 0.00166666666; //1.0f / 600
	normalMotionTime = 0.01666666666;// 1.0f / 60; 
	time = normalMotionTime;
	this->slowMotion = false;
	drawBomb = false;
	helpTextOn = true;
	s_pApp = this;

	this->SetWindowTitle("Benjamin Moore - RT3DT Assignment 2019");

	for (size_t i = 0; i < NUM_TEXTURE_FILES; ++i)
	{
		m_pTextures[i] = NULL;
		m_pTextureViews[i] = NULL;
	}

	m_pSamplerState = NULL;

	m_pHeightMap = NULL;

	m_pAeroplane = NULL;
	m_pAeroplaneDefaultMeshes = NULL;
	m_pAeroplaneShadowMeshes = NULL;

	m_pRenderTargetColourTexture = NULL;
	m_pRenderTargetDepthStencilTexture = NULL;
	m_pRenderTargetColourTargetView = NULL;
	m_pRenderTargetColourTextureView = NULL;
	m_pRenderTargetDepthStencilTargetView = NULL;

	m_drawHeightMapShaderVSConstants.pCB = NULL;
	m_drawHeightMapShaderPSConstants.pCB = NULL;

	m_pRenderTargetDebugDisplayBuffer = NULL;

	m_shadowCastingLightPosition = XMFLOAT3(4.0f, 3000.f, 100.f);
	m_shadowColour = XMFLOAT4(0.f, 0.f, 0.f, .25f);

	m_pShadowSamplerState = NULL;

	m_pTimes = NULL;
	m_fontStyle = CommonFont::Style(VertexColour(255, 0, 0, 255), XMFLOAT2(.09f, .09f));


	if (!this->CommonApp::HandleStart())
		return false;

	m_pTimes = CommonFont::CreateByName("Times", 14, 0, this);

	char aMaxNumLightsStr[100];
	_snprintf_s(aMaxNumLightsStr, sizeof aMaxNumLightsStr, _TRUNCATE, "%d", MAX_NUM_LIGHTS);

	const D3D_SHADER_MACRO aMacros[] = {
		{"MAX_NUM_LIGHTS", aMaxNumLightsStr, },
		{NULL},
	};

	if (!this->CompileShaderFromFile(&m_drawShadowCasterShader, "DrawShadowCaster.hlsl", aMacros, g_aVertexDesc_Pos3fColour4ubNormal3fTex2f, g_vertexDescSize_Pos3fColour4ubNormal3fTex2f))
		return false;

	// DrawHeightMap.hlsl
	{
		ID3D11VertexShader *pVS = NULL;
		ID3D11PixelShader *pPS = NULL;
		ID3D11InputLayout *pIL = NULL;		
		ShaderDescription vs, ps;

		for (size_t i = 0; i < NUM_TEXTURE_FILES; ++i)
		{
			LoadTextureFromFile(Application::s_pApp->GetDevice(), g_aTextureFileNames[i], &m_pTextures[i], &m_pTextureViews[i], &m_pSamplerState);
		}

		if (!CompileShadersFromFile(m_pD3DDevice, "DrawHeightMap.hlsl",
			"VSMain", &pVS, &vs,
			g_aVertexDesc_Pos3fColour4ubNormal3fTex2f, g_vertexDescSize_Pos3fColour4ubNormal3fTex2f, &pIL,
			"PSMain", &pPS, &ps,
			aMacros))
		{
			return false;
		}

		this->CreateShaderFromCompiledShader(&m_drawHeightMapShader, pVS, &vs, pIL, pPS, &ps);

		vs.FindCBuffer("DrawHeightMap", &m_drawHeightMapShaderVSConstants.slot);
		vs.FindFloat4x4(m_drawHeightMapShaderVSConstants.slot, "g_shadowMatrix", &m_drawHeightMapShaderVSConstants.shadowMatrix);
		vs.FindFloat4(m_drawHeightMapShaderVSConstants.slot, "g_shadowColour", &m_drawHeightMapShaderVSConstants.shadowColour);
		m_drawHeightMapShaderVSConstants.pCB = CreateBuffer(m_pD3DDevice, vs.GetCBufferSizeBytes(m_drawHeightMapShaderVSConstants.slot), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, NULL);

		ps.FindCBuffer("DrawHeightMap", &m_drawHeightMapShaderPSConstants.slot);
		ps.FindFloat4x4(m_drawHeightMapShaderPSConstants.slot, "g_shadowMatrix", &m_drawHeightMapShaderPSConstants.shadowMatrix);
		ps.FindFloat4(m_drawHeightMapShaderPSConstants.slot, "g_shadowColour", &m_drawHeightMapShaderPSConstants.shadowColour);
		m_drawHeightMapShaderPSConstants.pCB = CreateBuffer(m_pD3DDevice, ps.GetCBufferSizeBytes(m_drawHeightMapShaderPSConstants.slot), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, NULL);

		ps.FindTexture("g_texture0", &m_psTexture0);
		ps.FindTexture("g_texture1", &m_psTexture1);
		ps.FindTexture("g_texture2", &m_psTexture2);

		vs.FindTexture("g_materialMap", &m_vsMaterialMap);
		
		ps.FindTexture("g_shadowTexture", &m_heightMapShaderShadowTexture);
		ps.FindSamplerState("g_shadowSampler", &m_heightMapShaderShadowSampler);
		
	}

	if (!this->CreateRenderTarget())
		return false;

	m_bWireframe = false;

	
	/*mSpherePos = XMFLOAT4(0.0F, 30.4F, 0.0F, 0.0f);
	mSphereVel = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	mGravityAcc = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	mSphereCollided = false;*/
	m_pSphereMesh = CommonMesh::NewSphereMesh(this, 1.0f, 16, 16);
	m_pSphereShadowMesh = CommonMesh::NewSphereMesh(this, 1.0f, 16, 16);

	m_pHeightMap = new HeightMap( "Resources/heightmap.bmp", 2.0f, &m_drawHeightMapShader );
	m_pAeroplane = new Aeroplane( -140.0f, 3.5f, 36.0f, 105.0f );

	//m_pRobot = new Robot("hierarchy.txt", 0.0f, 2.4f, -20.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	//m_pRobot = new Robot("hierarchy.txt", 0.0f, 2.4f, 0.0f, 0.0f, 0.0f, 90.0f, 00.0f, 0.0f);
	m_pRobot = new Robot("hierarchy.txt", XMFLOAT4(-20.0F, 2.4F, 0.0F, 0.0F),
		XMFLOAT4(0.0F,90.0F,0.0F,0.0F),
		XMFLOAT4(2.0f, 41.0f, 51.0f, 0.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	m_pRobot1 = new Robot("hierarchy.txt", XMFLOAT4(0.0F, 2.4F, -20.0F, 0.0F),
		XMFLOAT4(0.0F, 00.0F, 0.0F, 0.0F),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	m_pRobot2 = new Robot("hierarchy.txt", XMFLOAT4(0.0F, 2.4F, 20.0F, 0.0F),
		XMFLOAT4(0.0F, 180.0F, 0.0F, 0.0F),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	m_pRobot3 = new Robot("hierarchy.txt", XMFLOAT4(20.0F, 2.4F, 0.0F, 0.0F),
		XMFLOAT4(0.0F, 270.0F, 0.0F, 0.0F),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));

	m_pRobot->LoadResources(m_pRobot);
	m_pRobot1->LoadResources(m_pRobot);
	m_pRobot2->LoadResources(m_pRobot);
	m_pRobot3->LoadResources(m_pRobot);

	m_pAeroplaneDefaultMeshes = AeroplaneMeshes::Load();
	if (!m_pAeroplaneDefaultMeshes)
		return false;

	// And the shadow copy.
	//
	// For a larger-scale project, with more complicated meshes, there would
    // probably be simplified meshes used for drawing the shadows. This example
    // just draws the same meshes using a different shader.
	m_pAeroplaneShadowMeshes = AeroplaneMeshes::Load();
	if (!m_pAeroplaneShadowMeshes)
		return false;

	//Change the shadow mesh collection to whatever
	m_pRobot->ChangeMeshToShadow(m_drawShadowCasterShader);
	m_pRobot1->ChangeMeshToShadow(m_drawShadowCasterShader);
	m_pRobot2->ChangeMeshToShadow(m_drawShadowCasterShader);
	m_pRobot3->ChangeMeshToShadow(m_drawShadowCasterShader);
	m_pSphereShadowMesh->SetShaderForAllSubsets(&m_drawShadowCasterShader);

	m_pAeroplaneShadowMeshes->pBulletMesh->SetShaderForAllSubsets(&m_drawShadowCasterShader);
	m_pAeroplaneShadowMeshes->pGunMesh->SetShaderForAllSubsets(&m_drawShadowCasterShader);
	m_pAeroplaneShadowMeshes->pPlaneMesh->SetShaderForAllSubsets(&m_drawShadowCasterShader);
	m_pAeroplaneShadowMeshes->pPropMesh->SetShaderForAllSubsets(&m_drawShadowCasterShader);
	m_pAeroplaneShadowMeshes->pTurretMesh->SetShaderForAllSubsets(&m_drawShadowCasterShader);
	

	m_cameraZ = 50.0f;
	m_rotationAngle = 0.f;
	m_rotationSpeed = 0.01f;

	m_cameraState = CAMERA_MAP;

	m_pShadowCastingLightMesh = CommonMesh::NewSphereMesh(this, 1.0f, 16, 16);
		
	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Application::HandleStop()
{
	Release(m_pRenderTargetDebugDisplayBuffer);

	Release(m_pRenderTargetColourTexture);
	Release(m_pRenderTargetDepthStencilTexture);
	Release(m_pRenderTargetColourTargetView);
	Release(m_pRenderTargetColourTextureView);
	Release(m_pRenderTargetDepthStencilTargetView);

	Release(m_pShadowSamplerState);
//	Release(m_pSamplerState);

	if (m_pSphereMesh)
		delete m_pSphereMesh;
	if (m_pSphereMesh)
		delete m_pSphereShadowMesh;


	Release(m_drawHeightMapShaderVSConstants.pCB);
	Release(m_drawHeightMapShaderPSConstants.pCB);

	delete m_pHeightMap;
	delete  m_pTimes;
	delete m_pAeroplane;
	delete m_pAeroplaneDefaultMeshes;
	delete m_pAeroplaneShadowMeshes;

	m_pRobot->ReleaseResources();
	delete m_pRobot;
	delete m_pRobot1;
	delete m_pRobot2;
	delete m_pRobot3;

	m_drawShadowCasterShader.Reset();

	delete m_pShadowCastingLightMesh;

	this->CommonApp::HandleStop();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Application::HandleUpdate()
{
	m_rotationAngle += m_rotationSpeed;

	if( m_cameraState == CAMERA_MAP || m_cameraState == CAMERA_ROBOT )
	{
		if (this->IsKeyPressed('Q'))
			m_cameraZ -= 2.0f;
		
		if (this->IsKeyPressed('A'))
			m_cameraZ += 2.0f;

		static bool dbS = false;

		if (this->IsKeyPressed(VK_SPACE))
		{
			if( !dbS )
			{
				if( m_rotationSpeed == 0.0f )
					m_rotationSpeed = 0.01f;
				else
					m_rotationSpeed = 0.0f;

				dbS = true;
			}
		}
		else
		{
			dbS = false;
		}
	}

	
	static bool dbC = false;

	if (this->IsKeyPressed('C') )	
	{
		if( !dbC )
		{
			if( ++m_cameraState == CAMERA_MAX )
				m_cameraState = CAMERA_MAP;

			dbC = true;
		}
	}
	else
	{
		dbC = false;
	}

	static bool dbH = false;
	if (this->IsKeyPressed('H'))
	{
		if (!dbH)
		{
			if (helpTextOn)
			{
				helpTextOn = false;
			}
			else
			{
				helpTextOn = true;
			}
			dbH = true;
		}
	}
	else
	{
		dbH = false;
	}

	if( m_cameraState != CAMERA_PLANE && m_cameraState != CAMERA_GUN)
	{
		if( this->IsKeyPressed(VK_LEFT) )
			m_shadowCastingLightPosition.x+=.5f;

		if( this->IsKeyPressed(VK_RIGHT) )
			m_shadowCastingLightPosition.x-=.5f;

		if( this->IsKeyPressed(VK_UP ) )
			m_shadowCastingLightPosition.z+=.5f;

		if( this->IsKeyPressed(VK_DOWN ) )
			m_shadowCastingLightPosition.z-=.5f;

		if( this->IsKeyPressed(VK_PRIOR ) )
			m_shadowCastingLightPosition.y-=.5f;

		if( this->IsKeyPressed(VK_NEXT ) )
			m_shadowCastingLightPosition.y+=.5f;
	}

	static bool dbW = false;
	if (this->IsKeyPressed('W') )	
	{
		if( !dbW )
		{
			m_bWireframe = !m_bWireframe;
			this->SetRasterizerState( false, m_bWireframe );
			dbW = true;
		}
	}
	else
	{
		dbW = false;
	}

	static bool dbS = false;
	if (Application::s_pApp->IsKeyPressed('S'))
	{
		if (!dbS)
		{
			if (slowMotion)
			{
				//time = slowMotionTime;
				slowMotion = false;
			}
			else
			{
				//time = normalMotionTime;
				slowMotion = true;
			}

			dbS = true;
		}

	}
	else
	{
		dbS = false;
	}
	
	static bool dbT = false;
	if (this->IsKeyPressed('B'))
	{
		if (dbT == false)
		{
			static int dx = 0;
			static int dy = 0;
			numberOfBounces = 0;
			m_cameraState = CAMERA_BOMB;
			mSpherePos = m_pAeroplane->GetWorldPositonFromMatrix();
			mSphereVel = m_pAeroplane->GetForwardVector();
			mGravityAcc = XMFLOAT4(0.0f, -0.05f, 0.0f, 0.0f);
			mSphereCollided = false;
			drawBomb = true;
			bombDropped = true;
			dbT = true;
		}
	}
	else
	{
		dbT = false;
	}

	XMVECTOR vSColPos, vSColNorm;
	if (bombDropped)
	{
		if (!mSphereCollided)
		{
			XMVECTOR vSPos = XMLoadFloat4(&mSpherePos);
			XMVECTOR vSVel = XMLoadFloat4(&mSphereVel);
			XMVECTOR vSAcc = XMLoadFloat4(&mGravityAcc);

			vSPos += vSVel; // Really important that we add LAST FRAME'S velocity as this was how fast the collision is expecting the ball to move
			vSVel += vSAcc; // The new velocity gets passed through to the collision so it can base its predictions on our speed NEXT FRAME


			XMStoreFloat4(&mSphereVel, vSVel);
			XMStoreFloat4(&mSpherePos, vSPos);


			mSphereSpeed = XMVectorGetX(XMVector4Length(vSVel));

			mSphereCollided = m_pHeightMap->RayCollision(vSPos, vSVel, mSphereSpeed, vSColPos, vSColNorm);

			if (mSphereCollided)
			{
			
				numberOfBounces++;
				vSVel ;
				XMStoreFloat4(&mSphereVel, XMVector4Reflect(vSVel, vSColNorm)); //basically allows us to bounce off a wall into the correct direction
				//mSphereVel = XMVector4Reflect(vSVel, vSColNorm);//mSphereVel.y * 0.8f;
				mSphereVel.x = mSphereVel.x * 0.7f;
				mSphereVel.y = mSphereVel.y * 0.7f;
				mSphereVel.z = mSphereVel.z * 0.7f;
				/*this will stop the ball going halfway into the ground and bring it up. Used to be one
				Had to change because when I added shadows in it was floating.*/
				mSpherePos.y = mSpherePos.y + 0.82f; 
				mSphereCollided = false;
				if (numberOfBounces > BOUNCE_LIMIT)
				{
					mSphereVel = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
					bombDropped = false;
				}

			}
		}
	}
	m_pAeroplane->Update( m_cameraState != CAMERA_MAP );

	static float slow;
	if (slowMotion == true)
	{
		slow += time;
		if (slow > 1)
		{
			m_pRobot->Update(time, m_pAeroplane);
			m_pRobot1->Update(time, m_pAeroplane);
			m_pRobot2->Update(time, m_pAeroplane);
			m_pRobot3->Update(time, m_pAeroplane);

			slow = 0;
		}
		
	}
	else
	{
		m_pRobot->Update(time, m_pAeroplane);
		m_pRobot1->Update(time, m_pAeroplane);
		m_pRobot2->Update(time, m_pAeroplane);
		m_pRobot3->Update(time, m_pAeroplane);
	}
	

	if (this->IsKeyPressed(VK_F1))
	{
		m_cameraState = CAMERA_ROBOT;
	}
	if (this->IsKeyPressed(VK_F2))
	{
		m_cameraState = CAMERA_PLANE;
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Application::HandleRender()
{
	this->RenderShadow();

	this->Render3D();

	this->Render2D();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void Application::RenderShadow()
{
	// Only the alpha channel is relevant, but clear the RGB channels to white
    // so that it's easy to see what's going on.
	float clearColour[4] = {0.f, 0.f, 0.f, 0.f};
	m_pD3DDeviceContext->ClearRenderTargetView(m_pRenderTargetColourTargetView, clearColour);
	m_pD3DDeviceContext->ClearDepthStencilView(m_pRenderTargetDepthStencilTargetView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	m_pD3DDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetColourTargetView, m_pRenderTargetDepthStencilTargetView);

	D3D11_VIEWPORT viewport = {0.f, 0.f, RENDER_TARGET_WIDTH, RENDER_TARGET_HEIGHT, 0.f, 1.f};
	m_pD3DDeviceContext->RSSetViewports(1, &viewport);

	//XMFLOAT4 vTemp = m_pAeroplane->GetPosition();
	//XMFLOAT4 vTemp = { 0.0f,0.0f,0.0f, 0.0f };
	XMFLOAT4 vTemp = m_pRobot->GetWorldPosition();
	XMVECTOR vPlanePos = XMLoadFloat4(&vTemp);

	//*************************************************************************
	/*float fovy = 0.8f;
	float zn = 1.0f;
	float zf = 1000.0f;
	float aspect = 1.2f;*/
	float aspect = RENDER_TARGET_WIDTH / RENDER_TARGET_HEIGHT;

	//disstancetorobot
	float distanceToPlane = sqrt(pow(vTemp.x - m_shadowCastingLightPosition.x, 2) + 
		pow(vTemp.y - m_shadowCastingLightPosition.y, 2) + 
		pow(vTemp.z - m_shadowCastingLightPosition.z, 2)); //pythagoras REMEMBER  THE CAMERA ON THE SLIDES IS THE LIGHTSOURCE.
	
	//float fovy = atan(AEROPLANE_RADIUS/distanceToPlane)* 2; //Sohcahtoa
	//float zn = distanceToPlane - AEROPLANE_RADIUS;
	//float zf = distanceToPlane + AEROPLANE_RADIUS;
	
	float fovy = atan(ROBOT_RADIUS/distanceToPlane)* 2; //Sohcahtoa
	float zn = distanceToPlane - ROBOT_RADIUS;
	float zf = distanceToPlane + ROBOT_RADIUS;
	//float aspect = RENDER_TARGET_WIDTH/ RENDER_TARGET_HEIGHT;

	//*************************************************************************
	if (zn <= 0.f)
	{
		zn = 0.1f;
	}
	if (zf <= 0.f)
	{
		zf = 0.1f;
	}
	XMMATRIX projMtx;
	projMtx = XMMatrixPerspectiveFovLH(fovy, aspect, zn, zf);
	this->SetProjectionMatrix(projMtx);

	XMMATRIX viewMtx;
	viewMtx = XMMatrixLookAtLH(XMLoadFloat3(&m_shadowCastingLightPosition), vPlanePos, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) );
	this->SetViewMatrix(viewMtx);

	m_shadowMtx = XMMatrixMultiply(viewMtx, projMtx );



	XMMATRIX worldMtx = XMMatrixIdentity();
	this->SetWorldMatrix(worldMtx);

	this->SetDepthStencilState(true);
	this->SetRasterizerState(false, false);
	
	
	m_pAeroplane->Draw(m_pAeroplaneShadowMeshes);
	m_pRobot->DrawShadow();
	m_pRobot1->DrawShadow();
	m_pRobot2->DrawShadow();
	m_pRobot3->DrawShadow();
	m_pRobot3->DrawShadow();

	if (drawBomb)
	{
		XMMATRIX worldMtxx;
		worldMtxx = XMMatrixTranslation(mSpherePos.x, mSpherePos.y, mSpherePos.z);
		this->SetWorldMatrix(worldMtxx);
		SetDepthStencilState(true, true);
		if (m_pSphereShadowMesh)
			m_pSphereShadowMesh->Draw();
	}
	this->SetDefaultRenderTarget();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void Application::Render3D()
{
	this->SetRasterizerState( false, m_bWireframe );

	this->SetDepthStencilState(true);

	XMVECTOR vCamera = XMVectorZero();
	XMVECTOR vLookat = XMVectorZero();;
    XMVECTOR vUpVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMFLOAT4 vPlanePos = m_pAeroplane->GetPosition();
	XMFLOAT4 vCamPos = m_pAeroplane->GetCameraPosition();
	XMFLOAT4 vFocusPos = m_pAeroplane->GetPosition();

	switch( m_cameraState )
	{
		case CAMERA_MAP:
			vCamera = XMVectorSet(sin(m_rotationAngle)*m_cameraZ, m_cameraZ/3 + 4, cos(m_rotationAngle)*m_cameraZ, 0.0f);
			vLookat = XMVectorSet(0.0f, 2.0f, 0.0f, 0.0f);
			break;
		case CAMERA_PLANE:
			m_pAeroplane->SetGunCamera( false );
			vCamera = XMLoadFloat4(&vCamPos);
			vLookat = XMLoadFloat4(&vFocusPos);
			break;
		case CAMERA_GUN:
			m_pAeroplane->SetGunCamera( true );
			vCamera = XMLoadFloat4(&vCamPos);
			vLookat = XMLoadFloat4(&vFocusPos);
			break;
		case CAMERA_ROBOT:
			vCamPos = m_pRobot->GetCameraPosition();
			vFocusPos = m_pRobot->GetFocusPosition();
			vCamera = XMLoadFloat4(&vCamPos);
			vLookat = XMLoadFloat4(&vFocusPos);
			break;
		case CAMERA_BOMB:
			vCamera = XMLoadFloat4(&vCamPos);
			vLookat = XMLoadFloat4(&mSpherePos);
			break;		
	}

    XMMATRIX  matView;
	matView = XMMatrixLookAtLH(vCamera, vLookat, vUpVector);

	XMMATRIX matProj;
	matProj = XMMatrixPerspectiveFovLH(kMath_PI / 4.f, 2, 1.5f, 5000.0f);

	this->SetViewMatrix(matView);
	this->SetProjectionMatrix(matProj);

	this->EnablePointLight(0, m_shadowCastingLightPosition, XMFLOAT3(1.0f, 1.0f, 1.0f) );
	this->SetLightAttenuation(0, 200.f, 2.f, 2.f, 2.f);
	this->EnableDirectionalLight(1, XMFLOAT3(-1.f, -1.f, -1.f), XMFLOAT3(0.65f, 0.55f, 0.65f));

	this->Clear(XMFLOAT4(.2f, .2f, .6f, 1.f));

	this->SetWorldMatrix(XMMatrixIdentity());
	
	{
		D3D11_MAPPED_SUBRESOURCE vsMap;
		if (!m_drawHeightMapShaderVSConstants.pCB || FAILED(m_pD3DDeviceContext->Map(m_drawHeightMapShaderVSConstants.pCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &vsMap)))
			vsMap.pData = NULL;

		D3D11_MAPPED_SUBRESOURCE psMap;
		if (!m_drawHeightMapShaderPSConstants.pCB || FAILED(m_pD3DDeviceContext->Map(m_drawHeightMapShaderPSConstants.pCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &psMap)))
			psMap.pData = NULL;

		SetCBufferFloat4x4(vsMap, m_drawHeightMapShaderVSConstants.shadowMatrix, m_shadowMtx);
		SetCBufferFloat4x4(psMap, m_drawHeightMapShaderPSConstants.shadowMatrix, m_shadowMtx);

		SetCBufferFloat4(vsMap, m_drawHeightMapShaderVSConstants.shadowColour, m_shadowColour);
		SetCBufferFloat4(psMap, m_drawHeightMapShaderPSConstants.shadowColour, m_shadowColour);

		if (psMap.pData)
			m_pD3DDeviceContext->Unmap(m_drawHeightMapShaderPSConstants.pCB, 0);

		if (vsMap.pData)
			m_pD3DDeviceContext->Unmap(m_drawHeightMapShaderVSConstants.pCB, 0);

		if (m_drawHeightMapShaderVSConstants.slot >= 0)
			m_pD3DDeviceContext->VSSetConstantBuffers(m_drawHeightMapShaderVSConstants.slot, 1, &m_drawHeightMapShaderVSConstants.pCB);

		if (m_drawHeightMapShaderPSConstants.slot >= 0)
			m_pD3DDeviceContext->PSSetConstantBuffers(m_drawHeightMapShaderPSConstants.slot, 1, &m_drawHeightMapShaderPSConstants.pCB);

		if (m_heightMapShaderShadowSampler >= 0)
			m_pD3DDeviceContext->PSSetSamplers(m_heightMapShaderShadowSampler, 1, &m_pShadowSamplerState);

		if (m_heightMapShaderShadowTexture >= 0)
			m_pD3DDeviceContext->PSSetShaderResources(m_heightMapShaderShadowTexture, 1, &m_pRenderTargetColourTextureView);

		if (m_psTexture0 >= 0)
			m_pD3DDeviceContext->PSSetShaderResources(m_psTexture0, 1, &m_pTextureViews[0]);

		if (m_psTexture1 >= 0)
			m_pD3DDeviceContext->PSSetShaderResources(m_psTexture1, 1, &m_pTextureViews[1]);

		if (m_psTexture2 >= 0)
			m_pD3DDeviceContext->PSSetShaderResources(m_psTexture2, 1, &m_pTextureViews[2]);

		if (m_vsMaterialMap >= 0)
			m_pD3DDeviceContext->VSSetShaderResources(m_vsMaterialMap, 1, &m_pTextureViews[3]);

		m_pSamplerState = Application::s_pApp->GetSamplerState(true, true, true);

		m_pHeightMap->Draw(m_pSamplerState);
	}
	

	

	m_pAeroplane->Draw(m_pAeroplaneDefaultMeshes);
	m_pRobot->DrawAll();
	m_pRobot1->DrawAll();
	m_pRobot2->DrawAll();
	m_pRobot3->DrawAll();
	if (drawBomb)
	{
		XMMATRIX worldMtxx;
		worldMtxx = XMMatrixTranslation(mSpherePos.x, mSpherePos.y, mSpherePos.z);
		this->SetWorldMatrix(worldMtxx);
		SetDepthStencilState(true, true);
		if (m_pSphereMesh)
			m_pSphereMesh->Draw();
	}
	XMMATRIX worldMtx = XMMatrixTranslation(m_shadowCastingLightPosition.x,  m_shadowCastingLightPosition.y,  m_shadowCastingLightPosition.z);
	this->SetWorldMatrix(worldMtx);
	m_pShadowCastingLightMesh->Draw();

	

	this->SetDefaultProjectionMatrix();

	this->SetDefaultViewMatrix(XMFLOAT3(0.f, 0.f, -70.f), XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(0.f, 1.f, 0.f));

	this->SetDepthStencilState(false);

	worldMtx = XMMatrixIdentity();
	this->SetWorldMatrix(worldMtx);

	
	this->SetWorldMatrix(worldMtx);

	if (helpTextOn)
	{
		m_pTimes->DrawString(XMFLOAT3(-28.f, 26.f, 0.f), &m_fontStyle, "H = Toggle screen help text");
		m_pTimes->DrawString(XMFLOAT3(-28.f, 24.f, 0.f), &m_fontStyle, "C = switch to the next camera in the scene.");
		m_pTimes->DrawString(XMFLOAT3(-28.f, 22.f, 0.f), &m_fontStyle, "F1: cut directly to a camera focussing on the animating robot.");
		m_pTimes->DrawString(XMFLOAT3(-28.f, 20.f, 0.f), &m_fontStyle, "F2: cut directly to a camera in the cockpit of the flying plane.");
		m_pTimes->DrawString(XMFLOAT3(-28.f, 18.f, 0.f), &m_fontStyle, "Q/A/O/P = up/down/left/right controls for flying the plane.");
		m_pTimes->DrawString(XMFLOAT3(-28.f, 16.f, 0.f), &m_fontStyle, "M = toggle the forward motion of plane (free roaming camera)");
		m_pTimes->DrawString(XMFLOAT3(-28.f, 14.f, 0.f), &m_fontStyle, "B = drop a bomb (automatically switches to bomb-cam)");
		m_pTimes->DrawString(XMFLOAT3(-28.f, 12.f, 0.f), &m_fontStyle, "Space = shoot");
		m_pTimes->DrawString(XMFLOAT3(-28.f, 10.f, 0.f), &m_fontStyle, "1/2/3 = Play robot animations (idle/attack/die)");
		m_pTimes->DrawString(XMFLOAT3(-28.f, 8.f, 0.f), &m_fontStyle, "S = slow motion mode for animations (1 frame per second).");
		m_pTimes->DrawString(XMFLOAT3(-28.f, 6.f, 0.f), &m_fontStyle, "First Camera = Q/A to zoom in and out, space bar to stop rotating");
		m_pTimes->DrawString(XMFLOAT3(-28.f, 4.f, 0.f), &m_fontStyle, "Second Camera = This is the aeroplane");
		m_pTimes->DrawString(XMFLOAT3(-28.f, 2.f, 0.f), &m_fontStyle, "Third Camera = This is the Robot, camera is stationary");
		m_pTimes->DrawString(XMFLOAT3(-28.f, 0.f, 0.f), &m_fontStyle, "Fourth Camera = Gun Camera");
		m_pTimes->DrawString(XMFLOAT3(-28.f, -2.f, 0.f), &m_fontStyle, "Fith Camera = This is the bomb, will be blank if no bomb has spawned");
		m_pTimes->DrawString(XMFLOAT3(-28.f, -4.f, 0.f), &m_fontStyle, "Fly near robots for them to attack you");
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void Application::Render2D()
{
	float windowWidth, windowHeight;
	this->GetWindowSize(&windowWidth, &windowHeight);

	XMMATRIX projMtx = XMMatrixOrthographicOffCenterLH(0.f, windowWidth, 0.f, windowHeight, 1.f, 250.f);
	this->SetProjectionMatrix(projMtx);

	XMMATRIX viewMtx = XMMatrixTranslation(0.f, 0.f, 2.f);
	this->SetViewMatrix(viewMtx);

	this->SetWorldMatrix(XMMatrixIdentity());
	
	this->SetDepthStencilState(false, false);
	this->SetRasterizerState(false, false);
	this->SetBlendState(false);

	//If we chnage the m_pRenderTargetDebugDisplayBuffer to null that map in bottom left goes.
	this->DrawTextured(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, NULL, NULL, 4, m_pRenderTargetColourTextureView, this->GetSamplerState());
	//this->DrawTextured(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, m_pRenderTargetDebugDisplayBuffer, NULL, 4, m_pRenderTargetColourTextureView, this->GetSamplerState());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool Application::CreateRenderTarget()
{
	// Create render target texture, and two views for it - one for using it as
    // a render target, and one for using it as a texture.
	{
		HRESULT hr;

		D3D11_TEXTURE2D_DESC td;

		td.Width = RENDER_TARGET_WIDTH;
		td.Height = RENDER_TARGET_HEIGHT;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;

		hr = m_pD3DDevice->CreateTexture2D(&td, NULL, &m_pRenderTargetColourTexture);
		if (FAILED(hr))
		{
			this->SetStartErrorMessage("Failed to create render target colour texture.");
			return false;
		}

		D3D11_RENDER_TARGET_VIEW_DESC rtvd;

		rtvd.Format = td.Format;
		rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvd.Texture2D.MipSlice = 0;

		hr = m_pD3DDevice->CreateRenderTargetView(m_pRenderTargetColourTexture, &rtvd, &m_pRenderTargetColourTargetView);
		if (FAILED(hr))
		{
			this->SetStartErrorMessage("Failed to create render target colour target view.");
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC svd;

		svd.Format = td.Format;
		svd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		svd.Texture2D.MostDetailedMip = 0;
		svd.Texture2D.MipLevels = 1;

		hr = m_pD3DDevice->CreateShaderResourceView(m_pRenderTargetColourTexture, &svd, &m_pRenderTargetColourTextureView);
		if (FAILED(hr))
		{
			this->SetStartErrorMessage("Failed to create render target colour texture view.");
			return false;
		}
	}

	// Do the same again, roughly, for its matching depth buffer. (There is a
	// default depth buffer, but it can't really be re-used for this, because it
	// tracks the window size and the window could end up too small.)
	{
		HRESULT hr;

		D3D11_TEXTURE2D_DESC td;

		td.Width = RENDER_TARGET_WIDTH;
		td.Height = RENDER_TARGET_HEIGHT;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_D16_UNORM;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;

		hr = m_pD3DDevice->CreateTexture2D(&td, NULL, &m_pRenderTargetDepthStencilTexture);
		if (FAILED(hr))
		{
			this->SetStartErrorMessage("Failed to create render target depth/stencil texture.");
			return false;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;

		dsvd.Format = td.Format;
		dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvd.Flags = 0;
		dsvd.Texture2D.MipSlice = 0;

		hr = m_pD3DDevice->CreateDepthStencilView(m_pRenderTargetDepthStencilTexture, &dsvd, &m_pRenderTargetDepthStencilTargetView);
		if (FAILED(hr))
		{
			this->SetStartErrorMessage("Failed to create render target depth/stencil view.");
			return false;
		}
	}

	// VB for drawing render target on screen.
	static const Vertex_Pos3fColour4ubTex2f aRenderTargetDebugDisplayBufferVtxs[] = {
		Vertex_Pos3fColour4ubTex2f(XMFLOAT3(0.f, 0.f, 0.f), VertexColour(255, 255, 255, 255), XMFLOAT2(0.f, 1.f)),
		Vertex_Pos3fColour4ubTex2f(XMFLOAT3(256.f, 0.f, 0.f), VertexColour(255, 255, 255, 255), XMFLOAT2(1.f, 1.f)),
		Vertex_Pos3fColour4ubTex2f(XMFLOAT3(0.f, 256.f, 0.f), VertexColour(255, 255, 255, 255), XMFLOAT2(0.f, 0.f)),
		Vertex_Pos3fColour4ubTex2f(XMFLOAT3(256.f, 256.f, 0.f), VertexColour(255, 255, 255, 255), XMFLOAT2(1.f, 0.f)),
	};

	m_pRenderTargetDebugDisplayBuffer = CreateImmutableVertexBuffer(m_pD3DDevice, sizeof aRenderTargetDebugDisplayBufferVtxs, aRenderTargetDebugDisplayBufferVtxs);

	// Sampler state for using the shadow texture to draw with.
	{
		D3D11_SAMPLER_DESC sd;

		sd.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;

		// Use BORDER addressing, so that anything outside the area the shadow
        // texture casts on can be given a specific fixed colour.
		sd.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sd.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sd.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;

		sd.MipLODBias = 0.f;
		sd.MaxAnisotropy = 16;
		sd.ComparisonFunc = D3D11_COMPARISON_NEVER;

		// Set the border colour to transparent, corresponding to unshadowed.
		sd.BorderColor[0] = 0.f;
		sd.BorderColor[1] = 0.f;
		sd.BorderColor[2] = 0.f;
		sd.BorderColor[3] = 0.f;

		sd.MinLOD = 0.f;
		sd.MaxLOD = D3D11_FLOAT32_MAX;

		if (FAILED(m_pD3DDevice->CreateSamplerState(&sd, &m_pShadowSamplerState)))
			return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{
	Application application;

	Run(&application);

	return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
