#include "HeightMap.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

HeightMap::HeightMap(char* filename, float gridSize, CommonApp::Shader *pShader)
{
	m_pShader = pShader;

	LoadHeightMap(filename, gridSize);

	m_pHeightMapBuffer = NULL;

	static const VertexColour MAP_COLOUR(200, 255, 255, 255);

	int vertex = 0;
	m_HeightMapVtxCount = (m_HeightMapLength - 1) * m_HeightMapWidth * 2;
	//m_HeightMapVtxCount = (m_HeightMapLength - 1)*(m_HeightMapWidth - 1) * 6;
	m_pMapVtxs = new Vertex_Pos3fColour4ubNormal3fTex2f[ m_HeightMapVtxCount ];
	
	int vtxIndex = 0;
	int mapIndex = 0;



	XMVECTOR v0, v1, v2, v3;
	XMVECTOR t0, t1, t2, t3;
	XMFLOAT3 v512 = XMFLOAT3(512.0f, 0.0f, 512.0f);
	XMVECTOR vOffset = XMLoadFloat3(&v512);

	for (int y = 0; y < m_HeightMapLength - 1; y++) //determines which row you are on
	{

		for (int x = 0; x < m_HeightMapWidth - 1; x++) //-1 to make it 255
		{
			if (y % 2 == 0) //Works out if the row is odd or even
			{
				mapIndex = (m_HeightMapLength * y) + x;

				v0 = XMLoadFloat3(&m_pHeightMap[mapIndex]);
				v1 = XMLoadFloat3(&m_pHeightMap[mapIndex + 1]);
				v2 = XMLoadFloat3(&m_pHeightMap[mapIndex + m_HeightMapWidth]);
				v3 = XMLoadFloat3(&m_pHeightMap[mapIndex + m_HeightMapWidth + 1]);


				VertexColour CUBE_COLOUR;

				t0 = (v0 + vOffset) / 32.0f;
				t1 = (v1 + vOffset) / 32.0f;
				t2 = (v2 + vOffset) / 32.0f;
				t3 = (v3 + vOffset) / 32.0f;

				t0 = XMVectorSwizzle(t0, 0, 2, 1, 3);
				t1 = XMVectorSwizzle(t1, 0, 2, 1, 3);
				t2 = XMVectorSwizzle(t2, 0, 2, 1, 3);
				t3 = XMVectorSwizzle(t3, 0, 2, 1, 3);

				XMVECTOR averageNormalsOne;
				XMVECTOR averageNormalsTwo;

				ReturnAverageNormal(mapIndex, averageNormalsOne);
				ReturnAverageNormal(mapIndex + m_HeightMapWidth, averageNormalsTwo);

				if (y == 0)
				{
					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3fTex2f(v2, MAP_COLOUR, averageNormalsTwo, t2);
					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3fTex2f(v0, MAP_COLOUR, averageNormalsOne,t0);
				}
				else
				{
					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3fTex2f(v2, MAP_COLOUR, averageNormalsTwo,t2);
					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3fTex2f(v0, MAP_COLOUR, averageNormalsOne,t0);
				}
			}
			else
			{
				mapIndex = (m_HeightMapLength * y) + (m_HeightMapWidth - x) - 2; //go backwards

				v0 = XMLoadFloat3(&m_pHeightMap[mapIndex]);
				v1 = XMLoadFloat3(&m_pHeightMap[mapIndex + 1]);
				v2 = XMLoadFloat3(&m_pHeightMap[mapIndex + m_HeightMapWidth]);
				v3 = XMLoadFloat3(&m_pHeightMap[mapIndex + m_HeightMapWidth + 1]);

				t0 = (v0 + vOffset) / 32.0f;
				t1 = (v1 + vOffset) / 32.0f;
				t2 = (v2 + vOffset) / 32.0f;
				t3 = (v3 + vOffset) / 32.0f;

				t0 = XMVectorSwizzle(t0, 0, 2, 1, 3);
				t1 = XMVectorSwizzle(t1, 0, 2, 1, 3);
				t2 = XMVectorSwizzle(t2, 0, 2, 1, 3);
				t3 = XMVectorSwizzle(t3, 0, 2, 1, 3);

				XMVECTOR averageNormalsOne;
				XMVECTOR averageNormalsTwo;

				ReturnAverageNormal(mapIndex, averageNormalsOne);
				ReturnAverageNormal(mapIndex + m_HeightMapWidth, averageNormalsTwo);
				

				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3fTex2f(v0, MAP_COLOUR, averageNormalsOne, t0);
				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3fTex2f(v2, MAP_COLOUR, averageNormalsTwo, t2);
			}
		

		}
	}

	m_pHeightMapBuffer = CreateImmutableVertexBuffer(Application::s_pApp->GetDevice(), sizeof Vertex_Pos3fColour4ubNormal3fTex2f * m_HeightMapVtxCount, m_pMapVtxs);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void HeightMap::ReturnAverageNormal(int mapindex, XMVECTOR &averagenormal)
{

	int y = mapindex / m_HeightMapWidth;
	int x = mapindex % m_HeightMapWidth;
	if (y == 255)
	{
		bool test = true;
	}
	if (x == 255)
	{
		bool test = true;
	}
	XMVECTOR faceNormalOne;
	XMVECTOR faceNormalTwo;
	XMVECTOR faceNormalThree;
	XMVECTOR faceNormalFour;
	XMVECTOR faceNormalFive;
	XMVECTOR faceNormalSix;
	XMVECTOR faceNormalAverage;

	if (y == 0 && x == 0) //top left corner
	{
		XMFLOAT3 v0 = m_pHeightMap[mapindex];  //Middle
		XMFLOAT3 v2 = m_pHeightMap[mapindex + m_HeightMapWidth]; //Middle bottom
		XMFLOAT3 v3 = m_pHeightMap[mapindex + 1];  //Middle right

		XMVECTOR vector0 = XMLoadFloat3(&XMFLOAT3(v0.x, v0.y, v0.z)); //middle
		XMVECTOR vector2 = XMLoadFloat3(&XMFLOAT3(v2.x, v2.y, v2.z)); // middle bottom
		XMVECTOR vector3 = XMLoadFloat3(&XMFLOAT3(v3.x, v3.y, v3.z)); //middle right

		faceNormalOne = XMVector3Cross(XMVectorSubtract(vector0, vector2), XMVectorSubtract(vector2, vector3)); // Middle, middle bottom, middle right //correct
		faceNormalAverage = faceNormalOne;
	}
	else if (y == 511 && x == 510) //bottom right corner
	{
		XMFLOAT3 v0 = m_pHeightMap[mapindex];  //Middle
		XMFLOAT3 v1 = m_pHeightMap[mapindex - m_HeightMapWidth]; //Middle top
		XMFLOAT3 v4 = m_pHeightMap[mapindex - 1];  //Middle left

		XMVECTOR vector0 = XMLoadFloat3(&XMFLOAT3(v0.x, v0.y, v0.z)); //middle
		XMVECTOR vector1 = XMLoadFloat3(&XMFLOAT3(v1.x, v1.y, v1.z)); // middle top
		XMVECTOR vector4 = XMLoadFloat3(&XMFLOAT3(v4.x, v4.y, v4.z)); //middle left

		faceNormalOne = XMVector3Cross(XMVectorSubtract(vector0, vector1), XMVectorSubtract(vector1, vector4)); // Middle, middle top, middle left //correct
		faceNormalAverage = faceNormalOne;
	}
	else if (y == 0 && x == 510) //top right corner
	{
		XMFLOAT3 v0 = m_pHeightMap[mapindex];  //Middle
		XMFLOAT3 v2 = m_pHeightMap[mapindex + m_HeightMapWidth]; //Middle bottom
		XMFLOAT3 v4 = m_pHeightMap[mapindex - 1];  //Middle left
		XMFLOAT3 v6 = m_pHeightMap[mapindex + m_HeightMapWidth - 1]; //Middle bottom left;

		XMVECTOR vector0 = XMLoadFloat3(&XMFLOAT3(v0.x, v0.y, v0.z)); //middle
		XMVECTOR vector2 = XMLoadFloat3(&XMFLOAT3(v2.x, v2.y, v2.z)); // middle bottom
		XMVECTOR vector4 = XMLoadFloat3(&XMFLOAT3(v4.x, v4.y, v4.z)); //middle left
		XMVECTOR vector6 = XMLoadFloat3(&XMFLOAT3(v6.x, v6.y, v6.z)); //middle bottom left

		faceNormalOne = XMVector3Cross(XMVectorSubtract(vector0, vector4), XMVectorSubtract(vector4, vector6)); //middle, middle left, middle bottom left // correct
		faceNormalTwo = XMVector3Cross(XMVectorSubtract(vector0, vector6), XMVectorSubtract(vector6, vector2)); //Middle, middle bottom left, middle bottom //correct
		faceNormalAverage = faceNormalOne + faceNormalTwo;
		faceNormalAverage = faceNormalAverage / 2;

	}
	else if (y == 511 && x == 0) //bottom left corner
	{
		XMFLOAT3 v0 = m_pHeightMap[mapindex];  //Middle
		XMFLOAT3 v1 = m_pHeightMap[mapindex - m_HeightMapWidth]; //Middle top
		XMFLOAT3 v3 = m_pHeightMap[mapindex + 1];  //Middle right
		XMFLOAT3 v5 = m_pHeightMap[mapindex - m_HeightMapWidth + 1]; //Middle top right;

		XMVECTOR vector0 = XMLoadFloat3(&XMFLOAT3(v0.x, v0.y, v0.z)); //middle
		XMVECTOR vector1 = XMLoadFloat3(&XMFLOAT3(v1.x, v1.y, v1.z)); // middle top
		XMVECTOR vector3 = XMLoadFloat3(&XMFLOAT3(v3.x, v3.y, v3.z)); //middle right
		XMVECTOR vector5 = XMLoadFloat3(&XMFLOAT3(v5.x, v5.y, v5.z)); //middle top right

		faceNormalOne = XMVector3Cross(XMVectorSubtract(vector0, vector3), XMVectorSubtract(vector3, vector5)); //Middle, middle right, middle top right //correct
		faceNormalTwo = XMVector3Cross(XMVectorSubtract(vector0, vector5), XMVectorSubtract(vector5, vector1)); //Middle, middle top right, middle top //correct
		faceNormalAverage = faceNormalOne + faceNormalTwo;
		faceNormalAverage = faceNormalAverage / 2;
	}
	else if (x == 0) //leftside
	{
		XMFLOAT3 v0 = m_pHeightMap[mapindex];  //Middle
		XMFLOAT3 v1 = m_pHeightMap[mapindex - m_HeightMapWidth]; //Middle top
		XMFLOAT3 v2 = m_pHeightMap[mapindex + m_HeightMapWidth]; //Middle bottom
		XMFLOAT3 v3 = m_pHeightMap[mapindex + 1];  //Middle right
		XMFLOAT3 v5 = m_pHeightMap[mapindex - m_HeightMapWidth + 1]; //Middle top right;

		XMVECTOR vector0 = XMLoadFloat3(&XMFLOAT3(v0.x, v0.y, v0.z)); //middle
		XMVECTOR vector1 = XMLoadFloat3(&XMFLOAT3(v1.x, v1.y, v1.z)); // middle top
		XMVECTOR vector2 = XMLoadFloat3(&XMFLOAT3(v2.x, v2.y, v2.z)); // middle bottom
		XMVECTOR vector3 = XMLoadFloat3(&XMFLOAT3(v3.x, v3.y, v3.z)); //middle right
		XMVECTOR vector5 = XMLoadFloat3(&XMFLOAT3(v5.x, v5.y, v5.z)); //middle top right

		faceNormalOne = XMVector3Cross(XMVectorSubtract(vector0, vector5), XMVectorSubtract(vector5, vector1)); //middle, middle top right, middle top; //correct
		faceNormalTwo = XMVector3Cross(XMVectorSubtract(vector0, vector3), XMVectorSubtract(vector3, vector5)); //Middle, middle right, middle top right //correct
		faceNormalThree = XMVector3Cross(XMVectorSubtract(vector0, vector2), XMVectorSubtract(vector2, vector3)); //Middle, middle bottom, middle right //correct
		faceNormalAverage = faceNormalOne + faceNormalTwo + faceNormalThree;
		faceNormalAverage = faceNormalAverage / 3;
	}
	else if (x == 510) //rightside
	{
		XMFLOAT3 v0 = m_pHeightMap[mapindex];  //Middle
		XMFLOAT3 v1 = m_pHeightMap[mapindex - m_HeightMapWidth]; //Middle top
		XMFLOAT3 v2 = m_pHeightMap[mapindex + m_HeightMapWidth]; //Middle bottom
		XMFLOAT3 v4 = m_pHeightMap[mapindex - 1];  //Middle left
		XMFLOAT3 v6 = m_pHeightMap[mapindex + m_HeightMapWidth - 1]; //Middle bottom left;

		XMVECTOR vector0 = XMLoadFloat3(&XMFLOAT3(v0.x, v0.y, v0.z)); //middle
		XMVECTOR vector1 = XMLoadFloat3(&XMFLOAT3(v1.x, v1.y, v1.z)); // middle top
		XMVECTOR vector2 = XMLoadFloat3(&XMFLOAT3(v2.x, v2.y, v2.z)); // middle bottom
		XMVECTOR vector4 = XMLoadFloat3(&XMFLOAT3(v4.x, v4.y, v4.z)); //middle left
		XMVECTOR vector6 = XMLoadFloat3(&XMFLOAT3(v6.x, v6.y, v6.z)); //middle bottom left

		faceNormalOne = XMVector3Cross(XMVectorSubtract(vector0, vector1), XMVectorSubtract(vector1, vector4)); //middle, middle top, middle left //correct
		faceNormalTwo = XMVector3Cross(XMVectorSubtract(vector0, vector4), XMVectorSubtract(vector4, vector6)); //Middle, middle left, middle bottom left //correct
		faceNormalThree = XMVector3Cross(XMVectorSubtract(vector0, vector6), XMVectorSubtract(vector6, vector2)); //Middle, middle bottom left, middle bottom //correct
		faceNormalAverage = faceNormalOne + faceNormalTwo + faceNormalThree;
		faceNormalAverage = faceNormalAverage / 3;
	}
	else if (y == 0) //topside
	{
		XMFLOAT3 v0 = m_pHeightMap[mapindex];  //Middle
		XMFLOAT3 v2 = m_pHeightMap[mapindex + m_HeightMapWidth]; //Middle bottom
		XMFLOAT3 v3 = m_pHeightMap[mapindex + 1];  //Middle right
		XMFLOAT3 v4 = m_pHeightMap[mapindex - 1];  //Middle left
		XMFLOAT3 v6 = m_pHeightMap[mapindex + m_HeightMapWidth - 1]; //Middle bottom left;

		XMVECTOR vector0 = XMLoadFloat3(&XMFLOAT3(v0.x, v0.y, v0.z)); //middle
		XMVECTOR vector2 = XMLoadFloat3(&XMFLOAT3(v2.x, v2.y, v2.z)); // middle bottom
		XMVECTOR vector3 = XMLoadFloat3(&XMFLOAT3(v3.x, v3.y, v3.z)); //middle right
		XMVECTOR vector4 = XMLoadFloat3(&XMFLOAT3(v4.x, v4.y, v4.z)); //middle left
		XMVECTOR vector6 = XMLoadFloat3(&XMFLOAT3(v6.x, v6.y, v6.z)); //middle bottom left

		faceNormalOne = XMVector3Cross(XMVectorSubtract(vector0, vector4), XMVectorSubtract(vector4, vector6)); //middle, middle left, middle bottom left //correct
		faceNormalTwo = XMVector3Cross(XMVectorSubtract(vector0, vector6), XMVectorSubtract(vector6, vector2)); //Middle, middle bottom left, middle bottom //correct
		faceNormalThree = XMVector3Cross(XMVectorSubtract(vector0, vector2), XMVectorSubtract(vector2, vector3)); //Middle, middle bottom, middle right //correct
		faceNormalAverage = faceNormalOne + faceNormalTwo + faceNormalThree;
		faceNormalAverage = faceNormalAverage / 3;
	}
	else if (y == 511) //bottom side
	{
		XMFLOAT3 v0 = m_pHeightMap[mapindex];  //Middle
		XMFLOAT3 v1 = m_pHeightMap[mapindex - m_HeightMapWidth]; //Middle top
		XMFLOAT3 v3 = m_pHeightMap[mapindex + 1];  //Middle right
		XMFLOAT3 v4 = m_pHeightMap[mapindex - 1];  //Middle left
		XMFLOAT3 v5 = m_pHeightMap[mapindex - m_HeightMapWidth + 1]; //Middle top right;

		XMVECTOR vector0 = XMLoadFloat3(&XMFLOAT3(v0.x, v0.y, v0.z)); //middle
		XMVECTOR vector1 = XMLoadFloat3(&XMFLOAT3(v1.x, v1.y, v1.z)); // middle top
		XMVECTOR vector3 = XMLoadFloat3(&XMFLOAT3(v3.x, v3.y, v3.z)); //middle right
		XMVECTOR vector4 = XMLoadFloat3(&XMFLOAT3(v4.x, v4.y, v4.z)); //middle left
		XMVECTOR vector5 = XMLoadFloat3(&XMFLOAT3(v5.x, v5.y, v5.z)); //middle top right

		faceNormalOne = XMVector3Cross(XMVectorSubtract(vector0, vector1), XMVectorSubtract(vector1, vector4)); //middle, middle top, middle left //correct
		faceNormalTwo = XMVector3Cross(XMVectorSubtract(vector0, vector5), XMVectorSubtract(vector5, vector1)); //Middle, middle top right, middle top //correct
		faceNormalThree = XMVector3Cross(XMVectorSubtract(vector0, vector3), XMVectorSubtract(vector3, vector5)); //Middle, middle right, middle top right //correct
		faceNormalAverage = faceNormalOne + faceNormalTwo + faceNormalThree;
		faceNormalAverage = faceNormalAverage / 3;
	}
	else // middle
	{
		XMFLOAT3 v0 = m_pHeightMap[mapindex];  //Middle
		XMFLOAT3 v1 = m_pHeightMap[mapindex - m_HeightMapWidth]; //Middle top
		XMFLOAT3 v2 = m_pHeightMap[mapindex + m_HeightMapWidth]; //Middle bottom
		XMFLOAT3 v3 = m_pHeightMap[mapindex + 1];  //Middle right
		XMFLOAT3 v4 = m_pHeightMap[mapindex - 1];  //Middle left
		XMFLOAT3 v5 = m_pHeightMap[mapindex - m_HeightMapWidth + 1]; //Middle top right;
		XMFLOAT3 v6 = m_pHeightMap[mapindex + m_HeightMapWidth - 1]; //Middle bottom left;

		XMVECTOR vector0 = XMLoadFloat3(&XMFLOAT3(v0.x, v0.y, v0.z)); //middle
		XMVECTOR vector1 = XMLoadFloat3(&XMFLOAT3(v1.x, v1.y, v1.z)); // middle top
		XMVECTOR vector2 = XMLoadFloat3(&XMFLOAT3(v2.x, v2.y, v2.z)); // middle bottom
		XMVECTOR vector3 = XMLoadFloat3(&XMFLOAT3(v3.x, v3.y, v3.z)); //middle right
		XMVECTOR vector4 = XMLoadFloat3(&XMFLOAT3(v4.x, v4.y, v4.z)); //middle left
		XMVECTOR vector5 = XMLoadFloat3(&XMFLOAT3(v5.x, v5.y, v5.z)); //middle top right
		XMVECTOR vector6 = XMLoadFloat3(&XMFLOAT3(v6.x, v6.y, v6.z)); //middle bottom left

		faceNormalOne = XMVector3Cross(XMVectorSubtract(vector0, vector5), XMVectorSubtract(vector5, vector1)); // Middle, middle top right, middle top // correct
		faceNormalTwo = XMVector3Cross(XMVectorSubtract(vector0, vector3), XMVectorSubtract(vector3, vector5)); // Middle, middle right, middle top right // correct
		faceNormalThree = XMVector3Cross(XMVectorSubtract(vector0, vector1), XMVectorSubtract(vector1, vector4)); // middle, middle top, middle left // correct
		faceNormalFour = XMVector3Cross(XMVectorSubtract(vector0, vector2), XMVectorSubtract(vector2, vector3)); // middle, middle bottom, middle right // correct
		faceNormalFive = XMVector3Cross(XMVectorSubtract(vector0, vector4), XMVectorSubtract(vector4, vector6)); //middle, middle left, middle bottom left // correct
		faceNormalSix = XMVector3Cross(XMVectorSubtract(vector0, vector6), XMVectorSubtract(vector6, vector2)); // middle, middle bottom left, middle bottom //correct

		faceNormalAverage = faceNormalOne + faceNormalTwo + faceNormalThree + faceNormalFour + faceNormalFive + faceNormalSix;
		faceNormalAverage = faceNormalAverage / 6;
	}

	averagenormal = -faceNormalAverage;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

HeightMap::~HeightMap()
{
	Release(m_pHeightMapBuffer);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void HeightMap::Draw(ID3D11SamplerState* samplerstate)
{
	CommonApp::Shader *pShader = m_pShader;
	if (!pShader)
		pShader = Application::s_pApp->GetUntexturedLitShader();

	Application::s_pApp->DrawWithShader(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, m_pHeightMapBuffer, sizeof(Vertex_Pos3fColour4ubNormal3fTex2f), NULL, 0, m_HeightMapVtxCount, NULL, samplerstate, pShader);

}

//////////////////////////////////////////////////////////////////////
// LoadHeightMap
// Original code sourced from rastertek.com
//////////////////////////////////////////////////////////////////////
bool HeightMap::LoadHeightMap(char* filename, float gridSize )
{
	FILE* filePtr;
	int error;
	unsigned int count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	int imageSize, i, j, k, index;
	unsigned char* bitmapImage;
	unsigned char height;


	// Open the height map file in binary.
	error = fopen_s(&filePtr, filename, "rb");
	if(error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Read in the bitmap info header.
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Save the dimensions of the terrain.
	m_HeightMapWidth = bitmapInfoHeader.biWidth;
	m_HeightMapLength = bitmapInfoHeader.biHeight;

	// Calculate the size of the bitmap image data.
	imageSize = m_HeightMapWidth * m_HeightMapLength * 3;

	// Allocate memory for the bitmap image data.
	bitmapImage = new unsigned char[imageSize];
	if(!bitmapImage)
	{
		return false;
	}

	// Move to the beginning of the bitmap data.
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Read in the bitmap image data.
	count = fread(bitmapImage, 1, imageSize, filePtr);
	if(count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if(error != 0)
	{
		return false;
	}

	// Create the structure to hold the height map data.
	XMFLOAT3* pUnsmoothedMap = new XMFLOAT3[m_HeightMapWidth * m_HeightMapLength];
	m_pHeightMap = new XMFLOAT3[m_HeightMapWidth * m_HeightMapLength];

	if(!m_pHeightMap)
	{
		return false;
	}

	// Initialize the position in the image data buffer.
	k=0;

	// Read the image data into the height map.
	for (j = m_HeightMapLength - 1; j >= 0; j--) {
		for (i = 0; i < m_HeightMapWidth; i++) {
			height = bitmapImage[k];
			index = (m_HeightMapLength * j) + i;
			m_pHeightMap[index].x = (float)(i - (m_HeightMapWidth / 2)) * gridSize;
			m_pHeightMap[index].y = (float)height / 16 * gridSize;
			m_pHeightMap[index].z = (float)((m_HeightMapLength / 2) - j) * gridSize;

			pUnsmoothedMap[index].y = (float)height / 6 * gridSize;
			k += 3;
		}
	}

	// Smoothing the landscape makes a big difference to the look of the shading
	for( int s=0; s<2; ++s )
	{
		for(j=0; j<m_HeightMapLength; j++)
		{
			for(i=0; i<m_HeightMapWidth; i++)
			{	
				index = (m_HeightMapWidth * j) + i;

				if( j>0 && j<m_HeightMapLength-1 && i>0 && i<m_HeightMapWidth-1 )
				{
					m_pHeightMap[index].y = 0.0f;
					m_pHeightMap[index].y += pUnsmoothedMap[index-m_HeightMapWidth-1].y	+ pUnsmoothedMap[index-m_HeightMapWidth].y + pUnsmoothedMap[index-m_HeightMapWidth+1].y;
					m_pHeightMap[index].y += pUnsmoothedMap[index-1].y	+ pUnsmoothedMap[index].y + pUnsmoothedMap[index+1].y;
					m_pHeightMap[index].y += pUnsmoothedMap[index+m_HeightMapWidth-1].y	+ pUnsmoothedMap[index+m_HeightMapWidth].y + pUnsmoothedMap[index+m_HeightMapWidth+1].y;
					m_pHeightMap[index].y /= 9;
				}		
			}
		}

		for(j=0; j<m_HeightMapLength; j++)
		{
			for(i=0; i<m_HeightMapWidth; i++)
			{	
				index = (m_HeightMapWidth * j) + i;
				pUnsmoothedMap[index].y = m_pHeightMap[index].y;
			}
		}

	}

	// Release the bitmap image data.
	delete [] bitmapImage;
	delete [] pUnsmoothedMap;
	bitmapImage = 0;

	return true;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


