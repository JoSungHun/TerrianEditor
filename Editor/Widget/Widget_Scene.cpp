#include "stdafx.h"
#include "Widget_Scene.h"
#include "../ImGui/ImGuizmo.h"
#include "./Scene/Actor.h"
#include "./Scene/Component/Camera.h"
#include "./Scene/Component/Transform.h"

Widget_Scene::Widget_Scene(Context * context)
	: IWidget(context)
	, frameTimer(0.0f)
{
	title = "Scene";
	windowFlags |=
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse;

	timer = context->GetSubsystem<Timer>();
	renderer = context->GetSubsystem<Renderer>();
}

void Widget_Scene::Render()
{
	ShowFrame();
	ShowTransformGizmo();
}

void Widget_Scene::ShowFrame()
{
	framePos.x = ImGui::GetCursorPos().x + ImGui::GetWindowPos().x;
	framePos.y = ImGui::GetCursorPos().y + ImGui::GetWindowPos().y;

	frameSize.x = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
	frameSize.y = ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y;

	frameSize.x -= static_cast<int>(frameSize.x) % 2 != 0 ? 1.0f : 0.0f;
	frameSize.y -= static_cast<int>(frameSize.y) % 2 != 0 ? 1.0f : 0.0f;

	ImVec2 mouse_pos = ImGui::GetMousePos() - framePos;

	if (frameTimer >= 0.1f)
	{
		//Log::InfoF("mouse position : %d, %d resolution %d, %d", static_cast<int>(mouse_pos.x), static_cast<int>(mouse_pos.y), static_cast<uint>(frameSize.x), static_cast<uint>(frameSize.y));
		renderer->SetCursorPos(D3DXVECTOR2(mouse_pos.x, mouse_pos.y));
		renderer->SetResolution
		(
			static_cast<uint>(frameSize.x),
			static_cast<uint>(frameSize.y)
		);
		frameTimer = 0.0f;
	}
	frameTimer += timer->GetDeltaTimeSec();

	ImGui::Image
	(
		renderer->GetFrameResource(),
		frameSize,
		ImVec2(0, 0),
		ImVec2(1, 1),
		ImVec4(1, 1, 1, 1),
		ImColor(50, 127, 166, 155)
	);
}

void Widget_Scene::ShowTransformGizmo()
{
	if (!EditorHelper::Get().GetSelectActor())
		return;

	auto camera = renderer->GetMainCamera();
	auto transform = EditorHelper::Get().GetSelectActor()->GetComponent<Transform>();

	if (!camera || !transform)
		return;

	static ImGuizmo::OPERATION operation(ImGuizmo::TRANSLATE);
	static ImGuizmo::MODE mode(ImGuizmo::WORLD);

	if (ImGui::IsKeyPressed(87)) // w
		operation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(69)) // e
		operation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(82)) // r
		operation = ImGuizmo::SCALE;

	auto matrix = transform->GetWorldMatrix();

	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(framePos.x, framePos.y, frameSize.x, frameSize.y);
	ImGuizmo::Manipulate
	(
		camera->GetViewMatrix(),
		camera->GetProjectionMatrix(),
		operation,
		mode,
		matrix
	);

	D3DXVECTOR3 position, scale;
	D3DXQUATERNION rotation;
	D3DXMatrixDecompose(&scale, &rotation, &position, &matrix);

	transform->SetTranslation(position);
	transform->SetRotation(rotation);
	transform->SetScale(scale);
}
