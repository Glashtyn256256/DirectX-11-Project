#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Application.h"

class Transform 
{
public:
	XMMATRIX CalculateLocalMatrix(const XMFLOAT4& skeletonpos, const XMFLOAT4& skeletonrot);
	XMMATRIX CalculateWorldMatrix(const XMMATRIX& m_mskeletonLocal, const XMMATRIX& m_mskeletonparent, const XMMATRIX& m_mskeletonworld);
	XMMATRIX CalculateWorldMatrix(const XMMATRIX& m_mskeletonparent, const XMMATRIX& m_mskeletonLocal);

private:
	
};
#endif
