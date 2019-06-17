#include "stdafx.h"
#include "EditorHelper.h"
#include "./Scene/Actor.h"

void EditorHelper::Initialize(Context * context)
{
	this->context = context;
	this->resourceManager = context->GetSubsystem<ResourceManager>();
	this->sceneManager = context->GetSubsystem<SceneManager>();
	this->thread = context->GetSubsystem<Thread>();
	this->renderer = context->GetSubsystem<Renderer>();
}

//void EditorHelper::LoadModel(const std::string & path) const
//{
//	thread->AddTask([this, path]()
//	{
//		resourceManager->Load<Model>(path);
//	});
//}
