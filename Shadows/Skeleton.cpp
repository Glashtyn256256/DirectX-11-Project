#include "Skeleton.h"



Skeleton::Skeleton(){};

Skeleton::~Skeleton(void)
{
}

//void Skeleton::LoadResource(std::string foldername)
//{
//	foldername = "Resources/" + foldername + "/" + partName + ".x";
//	s_pSkeletonBodyPartMesh = CommonMesh::LoadFromXFile(Application::s_pApp, foldername.c_str());
//}
//
//void Skeleton::ReleaseResource(void)
//{
//	delete s_pSkeletonBodyPartMesh;
//}
//
//void Skeleton::Draw(void)
//{
//	Application::s_pApp->SetWorldMatrix(m_mWorldMatrix); //this would be worked out through the transform class
//	s_pSkeletonBodyPartMesh->Draw();
//	
//}