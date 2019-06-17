#pragma once
#include "IWidget.h"

class Widget_Inspector final : public IWidget
{
public:
	Widget_Inspector(class Context* context);
    ~Widget_Inspector() = default;

	Widget_Inspector(const Widget_Inspector&) = delete;
	Widget_Inspector& operator=(const Widget_Inspector&) = delete;

	void Render() override;

private:
    void ShowTransform(class Transform* transform);
    void ShowLight(class Light* light);
    void ShowCamera(class Camera* camera);
    void ShowRenderable(class Renderable* renderable);
    void ShowMaterial(class Material* material);

private:
    void ShowAddComponentButton();
    void ShowComponentPopup();
};