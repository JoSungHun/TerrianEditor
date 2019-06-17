#pragma once
#include "IWidget.h"

class Widget_Hierarchy final : public IWidget
{
public:
	Widget_Hierarchy(class Context* context);
    ~Widget_Hierarchy() = default;

	Widget_Hierarchy(const Widget_Hierarchy&) = delete;
	Widget_Hierarchy& operator=(const Widget_Hierarchy&) = delete;

	void Render() override;

private:
    void ShowHierarchy();
    void AddScene(class Scene* scene);
    void AddActor(class Actor* actor);

private:
    void ShowPopup();

private:
    auto CreateEmpty()-> class Actor*;
    void CreateStandardMesh(const MeshType& type);

private:
    class SceneManager* sceneMgr;
};