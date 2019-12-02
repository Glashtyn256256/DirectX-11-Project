#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

//**********************************************************************
// File:			HeightMap.h
// Description:		A simple class to represent a heightmap
// Module:			Real-Time 3D Techniques for Games
// Created:			Jake - 2010-2011
// Notes:			
//**********************************************************************

#include "Application.h"

class HeightMap
{
public:
	// If pShader is NULL, a default shader will be used.
	HeightMap(char* filename, float gridSize, CommonApp::Shader *pShader);
	~HeightMap();

	void Draw(ID3D11SamplerState* samplerstate);
	bool RayCollision(XMVECTOR & rayPos, XMVECTOR rayDir, float raySpeed, XMVECTOR & colPos, XMVECTOR & colNormN);

private:
	bool LoadHeightMap(char* filename, float gridSize);

	bool PointOverQuad(XMVECTOR & vPos, XMVECTOR & v0, XMVECTOR & v1, XMVECTOR & v2);

	

	bool RayTriangle(const XMVECTOR & vert0, const XMVECTOR & vert1, const XMVECTOR & vert2, const XMVECTOR & rayPos, const XMVECTOR & rayDir, XMVECTOR & colPos, XMVECTOR & colNormN, float & colDist);

	bool PointPlane(const XMVECTOR & vert0, const XMVECTOR & vert1, const XMVECTOR & vert2, const XMVECTOR & pointPos);

	ID3D11Buffer *m_pHeightMapBuffer;

	void ReturnAverageNormal(int mapindex, XMVECTOR & averagenormal);
	int m_HeightMapWidth;
	int m_HeightMapLength;
	int m_HeightMapVtxCount;
	int m_HeightMapFaceCount;
	int m_FacesPerRow;
	XMFLOAT3* m_pHeightMap;
	Vertex_Pos3fColour4ubNormal3fTex2f* m_pMapVtxs;
	CommonApp::Shader *m_pShader;

};

#endif