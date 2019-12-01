#ifndef SKELETON_H
#define SKELETON_H

#include "Robot.h"
#include "Transform.h"


__declspec(align(16)) class Skeleton
{
public:
	Skeleton();
	~Skeleton(void);

	std::string GetPartName(void) { return partName; }
	std::string GetParentName(void) { return parentName; }

	float GetOffsetXPosition(void) { return m_v4SkeletonPositionOffset.x; }
	float GetOffsetYPosition(void) { return m_v4SkeletonPositionOffset.y; }
	float GetOffsetZPosition(void) { return m_v4SkeletonPositionOffset.z; }
	XMFLOAT4 GetOffsetPosition(void) { return m_v4SkeletonPositionOffset; }

	float GetRotationXPosition(void) { return m_v4SkeletonRotation.x; }
	float GetRotationYPosition(void) { return m_v4SkeletonRotation.y; }
	float GetRotationZPosition(void) { return m_v4SkeletonRotation.z; }
	XMFLOAT4 GetRotationPosition(void) { return m_v4SkeletonRotation; }

	XMMATRIX GetLocalMatrix() { return m_mSkeletonLocalMatrix; }
	XMMATRIX GetWorldMatrix() { return m_mSkeletonWorldMatrix; }
	
	void SetLocalMatrix() {
		m_mSkeletonLocalMatrix = transform.CalculateLocalMatrix(this->m_v4SkeletonPositionOffset, this->m_v4SkeletonRotation);
	}

	void SetWorldMatrix(XMMATRIX parentsworldmatrix) 
	{
		m_mSkeletonWorldMatrix = transform.CalculateWorldMatrix(this->m_mSkeletonLocalMatrix, parentsworldmatrix);
	}

	void SetPartName(std::string partname) {
		partName = partname;
	}

	void SetParentName(std::string parentname) {
		parentName = parentname;
	}

	void SetSkeletonOffsetPosition(float x, float y, float z, float w)
	{
		m_v4SkeletonPositionOffset.x = x;
		m_v4SkeletonPositionOffset.y = y;
		m_v4SkeletonPositionOffset.z = z;
		m_v4SkeletonPositionOffset.w = w;
	}

	void SetSkeletonOffsetPosition(XMFLOAT4 pos)
	{
		m_v4SkeletonPositionOffset = pos;
	}

	void SetSkeletonRotationPosition(XMFLOAT4 rot)
	{
		m_v4SkeletonRotation = rot;
	}

	void SetSkeletonRotationPosition(float x, float y, float z, float w)
	{
		m_v4SkeletonRotation.x = x;
		m_v4SkeletonRotation.y = y;
		m_v4SkeletonRotation.z = z;
		m_v4SkeletonRotation.w = w;
	}

private:
	std::string partName;
	std::string parentName;
	XMFLOAT4 m_v4SkeletonRotation; // Euler rotation angles
	XMFLOAT4 m_v4SkeletonPositionOffset; // offset
	XMMATRIX m_mSkeletonLocalMatrix = XMMatrixIdentity();
	XMMATRIX m_mSkeletonWorldMatrix = XMMatrixIdentity();
	Transform transform;
	

public:
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}

};
#endif

