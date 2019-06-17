#include "stdafx.h"
#include "Widget_Hierarchy.h"
#include "./Scene/Scene.h"
#include "./Scene/Actor.h"
#include "./Scene/Component/Renderable.h"

Widget_Hierarchy::Widget_Hierarchy(Context * context)
	: IWidget(context)
{
	title = "Hierarchy";
    sceneMgr = context->GetSubsystem<SceneManager>();
}

void Widget_Hierarchy::Render()
{
    if (!bVisible)
        return;

    ShowHierarchy();

    if (ImGui::IsWindowHovered())
    {
        if (ImGui::GetIO().MouseDown[1])
            ImGui::OpenPopup("Hierarchy Popup");
    }

    ShowPopup();

}

void Widget_Hierarchy::ShowHierarchy()
{
    auto scenes = sceneMgr->GetScenes();
    for (const auto& scene : scenes)
        AddScene(scene.second);
}

void Widget_Hierarchy::AddScene(Scene * scene)
{
    if (!scene)
        return;

    auto actors = scene->GetActors();
    
    if (ImGui::CollapsingHeader(scene->GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (auto& actor : actors)
        {
            if (!actor->GetTransform()->HasParent())
                AddActor(actor);

            if (ImGui::IsItemClicked())
                EditorHelper::Get().SetSelectActor(actor);
        }
    }
}

void Widget_Hierarchy::AddActor(Actor * actor)
{
    if (!actor)
        return;

    auto childs = actor->GetTransform()->GetChilds();
    auto flags = childs.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_OpenOnArrow;

    if (ImGui::TreeNodeEx(actor->GetName().c_str(), flags))
    {
        for (auto& child : childs)
        {
            AddActor(child->GetActor());
            
            if (ImGui::IsItemClicked())
                EditorHelper::Get().SetSelectActor(child->GetActor());
        }
        ImGui::TreePop();
    }
}

void Widget_Hierarchy::ShowPopup()
{
    if (ImGui::BeginPopup("Hierarchy Popup"))
    {
        if (ImGui::MenuItem("Copy")) {}
        if (ImGui::MenuItem("Delete")) {}

        ImGui::Separator();

        if (ImGui::MenuItem("Empty Actor")) {}

        if (ImGui::BeginMenu("Create 2D"))
        {
            if (ImGui::MenuItem("Quad"))        CreateStandardMesh(MeshType::Quad);
            if (ImGui::MenuItem("ScreenQuad"))  CreateStandardMesh(MeshType::ScreenQuad);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Create 3D"))
        {
            if (ImGui::MenuItem("Empty"))       CreateEmpty();
            if (ImGui::MenuItem("Cube"))        CreateStandardMesh(MeshType::Cube);
            if (ImGui::MenuItem("Cylinder"))    CreateStandardMesh(MeshType::Cylinder);
            if (ImGui::MenuItem("Cone"))        CreateStandardMesh(MeshType::Cone);
            if (ImGui::MenuItem("Capsule"))     CreateStandardMesh(MeshType::Capsule);
            if (ImGui::MenuItem("Sphere"))      CreateStandardMesh(MeshType::Sphere);

            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }
}

auto Widget_Hierarchy::CreateEmpty() -> class Actor *
{
    auto scene = sceneMgr->GetCurrentScene();
    if (scene)
    {
        auto actor = scene->CreateActor();
        actor->SetName("Object");
        
        return actor;
    }
    return nullptr;
}

void Widget_Hierarchy::CreateStandardMesh(const MeshType & type)
{
    auto actor = CreateEmpty();
    if (actor)
    {
        auto renderable = actor->AddComponent<Renderable>();
        renderable->SetStandardMaterial();
        renderable->SetStandardMesh(type);

        switch (type)
        {
        case MeshType::Custom:                                      break;
        case MeshType::Cube:        actor->SetName("Cube");         break;
        case MeshType::Cylinder:    actor->SetName("Cylinder");     break;
        case MeshType::Cone:        actor->SetName("Cone");         break;
        case MeshType::Capsule:     actor->SetName("Capsule");      break;
        case MeshType::Sphere:      actor->SetName("Sphere");       break;
        case MeshType::Quad:        actor->SetName("Quad");         break;
        case MeshType::ScreenQuad:  actor->SetName("ScreenQuad");   break;
        default:                                                    break;
        }
    }
}