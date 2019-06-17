#include "stdafx.h"
#include "Widget_Inspector.h"
#include "./Scene/Actor.h"
#include "./Scene/Component/Camera.h"
#include "./Scene/Component/Transform.h"
#include "./Scene/Component/Light/Light.h"
#include "./Scene/Component/Renderable.h"

Widget_Inspector::Widget_Inspector(Context * context)
	: IWidget(context)
{
	title = "Inspector";
}

void Widget_Inspector::Render()
{
    if (!bVisible)
        return;

    if (auto actor = EditorHelper::Get().GetSelectActor())
    {
        ShowTransform(actor->GetComponent<Transform>());
        ShowLight(actor->GetComponent<Light>());
        ShowCamera(actor->GetComponent<Camera>());
        ShowRenderable(actor->GetComponent<Renderable>());
    }   

    ShowAddComponentButton();
    ShowComponentPopup();
}

void Widget_Inspector::ShowTransform(Transform * transform)
{
    if (!transform)
        return;

    auto position   = transform->GetTranslation();
    auto rotation   = transform->GetEulerAngle();
    auto scale      = transform->GetScale();

    auto ShowFloat = [](const char* label, float* value)
    {
        float step = 1.0f;
        float step_fast = 1.0f;
        char* format = const_cast<char*>("%.3f");
        auto flags = ImGuiInputTextFlags_CharsDecimal;

        ImGui::PushItemWidth(100.0f);
        ImGui::InputFloat(label, value, step, step_fast, format, flags);
        ImGui::PopItemWidth();
    };

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Position");
        ImGui::SameLine(70.0f); ShowFloat("##PositionX", &position.x);
        ImGui::SameLine();      ShowFloat("##PositionY", &position.y);
        ImGui::SameLine();      ShowFloat("##PositionZ", &position.z);

        ImGui::Text("Rotation");
        ImGui::SameLine(70.0f); ShowFloat("##RotationX", &rotation.x);
        ImGui::SameLine();      ShowFloat("##RotationY", &rotation.y);
        ImGui::SameLine();      ShowFloat("##RotationZ", &rotation.z);

        ImGui::Text("Scale");
        ImGui::SameLine(70.0f); ShowFloat("##ScaleX", &scale.x);
        ImGui::SameLine();      ShowFloat("##ScaleY", &scale.y);
        ImGui::SameLine();      ShowFloat("##ScaleZ", &scale.z);
    }

    transform->SetTranslation(position);
    transform->SetRotationFromEulerAngle(rotation);
    transform->SetScale(scale);
}

void Widget_Inspector::ShowLight(Light * light)
{
    if (!light)
        return;

    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
		static std::vector<std::string> light_types{ "Directional", "Point", "Spot" };
        std::string light_type  = light_types[static_cast<uint>(light->GetLightType())];
        auto intensity          = light->GetIntensity();
        auto angle              = Math::ToDegree(light->GetAngle());
        auto cast_shadow        = light->IsCastShadow();
        auto bias               = light->GetBias();
        auto normal_bias        = light->GetNormalBias();
        auto range              = light->GetRange();
        auto color              = light->GetColor();

        //Type
        ImGui::TextUnformatted("Type");
        ImGui::PushItemWidth(110.0f);
        ImGui::SameLine(70.0f);
        if (ImGui::BeginCombo("##LightType", light_type.c_str()))
        {
            for (uint i = 0; i < light_types.size(); i++)
            {
                const auto bSelected = (light_type == light_types[i]);
                if (ImGui::Selectable(light_types[i].c_str(), bSelected))
                {
                    light_type = light_types[i];
                    light->SetLightType(static_cast<LightType>(i));
                }

                if (bSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();

        //Color
        ImGui::TextUnformatted("Color");
        ImGui::SameLine(70.0f);
        ImGui::ColorEdit4("##LightColor", color);

        //Intensity
        ImGui::Text("Intensity");
        ImGui::SameLine(70.0f);
        ImGui::PushItemWidth(300.0f);
        ImGui::DragFloat("##LightIntensity", &intensity, 0.01f, 0.0f, 100.0f);
        ImGui::PopItemWidth();
        
        //Cast Shadow
        ImGui::TextUnformatted("Shadow");
        ImGui::SameLine(70.0f);
        ImGui::Checkbox("##LightShadow", &cast_shadow);

        //Cascade
        if (light->GetLightType() == LightType::Directional)
        {
            //Bias
            ImGui::TextUnformatted("Bias");
            ImGui::SameLine(70.0f);
            ImGui::PushItemWidth(300.0f);
            ImGui::InputFloat("##LightBias", &bias, 0.0001f, 0.0001f, "%.4f");
            ImGui::PopItemWidth();

            //Normal Bias
            ImGui::TextUnformatted("Normal Bias");
            ImGui::SameLine(70.0f);
            ImGui::PushItemWidth(300.0f);
            ImGui::InputFloat("##LightNormalBias", &normal_bias, 1.0f, 1.0f, "%.0f");
            ImGui::PopItemWidth();
        }

        //Angle
        if (light->GetLightType() == LightType::Spot)
        {
            ImGui::TextUnformatted("Angle");
            ImGui::SameLine(70.0f);
            ImGui::PushItemWidth(300.0f);
            ImGui::DragFloat("##LightAngle", &angle, 0.01f, 1.0f, 179.0f);
            ImGui::PopItemWidth();
        }

        //Range
        if (light->GetLightType() != LightType::Directional)
        {
            ImGui::TextUnformatted("Range");
            ImGui::SameLine(70.0f);
            ImGui::PushItemWidth(300.0f);
            ImGui::DragFloat("##LightRange", &range, 0.01f, 0.0f, 100.0f);
            ImGui::PopItemWidth();
        }

        if (intensity != light->GetIntensity())         light->SetIntensity(intensity);
        if (cast_shadow != light->IsCastShadow())       light->SetIsCastShadow(cast_shadow);
        if (Math::ToRadian(angle) != light->GetAngle()) light->SetAngle(Math::ToRadian(angle));
        if (range != light->GetRange())                 light->SetRange(range);
        if (bias != light->GetBias())                   light->SetBias(bias);
        if (normal_bias != light->GetNormalBias())      light->SetNormalBias(normal_bias);
        if (color != light->GetColor())                 light->SetColor(color);
    }
}

void Widget_Inspector::ShowCamera(Camera * camera)
{
    if (!camera)
        return;

    auto projectionType = camera->GetProjectionType();
    const char* projectionNames[] = { "Perspective", "Orthographic" };
    const char* projectionName = projectionNames[static_cast<uint>(projectionType)];

    auto fov = camera->GetFOV();
    auto nearPlane = camera->GetNearPlane();
    auto farPlane = camera->GetFarPlane();

    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushItemWidth(110.0f);
        {
            ImGui::Text("Projection");
            ImGui::SameLine(140.0f);

            if (ImGui::BeginCombo("##ProjectionType", projectionName))
            {
                for (uint i = 0; i < IM_ARRAYSIZE(projectionNames); i++)
                {
                    bool bSelected = projectionName == projectionNames[i];
                    if (ImGui::Selectable(projectionNames[i], bSelected))
                        projectionName = projectionNames[i];

                    if (bSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::Text("FOV");
            ImGui::SameLine(140.0f);
            ImGui::SliderFloat("##FOV", &fov, 1.0f, 179.0f);

            ImGui::Text("Near");
            ImGui::SameLine(140.0f);
            ImGui::InputFloat("##Near", &nearPlane, 0.1f, 0.1f, "%.3f", ImGuiInputTextFlags_CharsDecimal);
        
            ImGui::Text("Far");
            ImGui::SameLine(140.0f);
            ImGui::InputFloat("##Far", &farPlane, 0.1f, 0.1f, "%.3f", ImGuiInputTextFlags_CharsDecimal);

        }
        ImGui::PopItemWidth();
    }

    //camera->SetProjectionType();
    camera->SetFOV(fov);
    camera->SetNearPlane(nearPlane);
    camera->SetFarPlane(farPlane);
}


void Widget_Inspector::ShowRenderable(Renderable * renderable)
{
    if (!renderable)
        return;

    auto material   = renderable->GetMaterial();
    auto mesh       = renderable->GetMesh();

    if (ImGui::CollapsingHeader("Renderable", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto materialName   = material ? material->GetResourceName() : "N/A";
        auto meshName       = mesh ? mesh->GetResourceName() : "N/A";
        
        ImGui::Text("Mesh");
        ImGui::SameLine(140.0f);
        ImGui::InputText("##Mesh", &meshName, ImGuiInputTextFlags_ReadOnly);
        ImGui::SameLine();
        if (ImGui::Button("Edit"))
        {
            //TODO : 
        }

        ImGui::Text("Material");
        ImGui::SameLine(140.0f);
        ImGui::InputText("##Material", &materialName, ImGuiInputTextFlags_ReadOnly);
        ImGui::SameLine();
        if (ImGui::Button("Edit"))
        {
            //TODO : 
        }
    }

    ShowMaterial(material);
}

void Widget_Inspector::ShowMaterial(Material * material)
{
    if (!material)
        return;

    if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto roughness      = material->GetRoughnessMultiplier();
        auto metallic       = material->GetMetallicMultiplier();
        auto normal         = material->GetNormalMultiplier();
        auto height         = material->GetHeightMultiplier();
        auto tiling         = material->GetTiling();
        auto offset         = material->GetOffset();
        auto colorAlbedo    = material->GetColorAlbedo();

        const auto albedoTexture    = material->GetTexture(TextureType::Albedo);
        const auto roughnessTexture = material->GetTexture(TextureType::Roughness);
        const auto metallicTexture  = material->GetTexture(TextureType::Metallic);
        const auto normalTexture    = material->GetTexture(TextureType::Normal);
        const auto heightTexture    = material->GetTexture(TextureType::Height);
        const auto occlusionTexture = material->GetTexture(TextureType::Occlusion);
        const auto emissiveTexture  = material->GetTexture(TextureType::Emissive);
        const auto maskTexture      = material->GetTexture(TextureType::Mask);

        const auto ShowTextureSlot = [&material](Texture* texture, const char* name, const TextureType& type) 
        {
            ImGui::Text(name);
            ImGui::SameLine(70.0f);
            ImGui::Image
            (
                texture ? texture->GetShaderResourceView() : nullptr,
                ImVec2(80, 80),
                ImVec2(0, 0),
                ImVec2(1, 1),
                ImVec4(1, 1, 1, 1),
                ImColor(255, 255, 255, 128)
            );

            //DragDrop
            auto data = DragDrop::GetDragDrop(DragDropPayloadType::Texture);
            if (data != "")
                material->SetTexture(type, data);
        };

        //Albedo
        ShowTextureSlot(albedoTexture, "Albedo", TextureType::Albedo);
        ImGui::Text("Color"); 
        ImGui::SameLine(70.0f);
        ImGui::ColorEdit4("##ColorAlbedo", colorAlbedo);

        ImGui::Separator();

        //Roughness
        ShowTextureSlot(roughnessTexture, "Roughness", TextureType::Roughness);
        ImGui::Text("Factor");
        ImGui::SameLine(70.0f);
        ImGui::SliderFloat("##Roughness", &roughness, 0.0f, 1.0f);

        ImGui::Separator();

        //Metallic
        ShowTextureSlot(metallicTexture, "Metallic", TextureType::Metallic);
        ImGui::Text("Factor");
        ImGui::SameLine(70.0f);
        ImGui::SliderFloat("##Metallic", &metallic, 0.0f, 1.0f);

        ImGui::Separator();

        //Normal
        ShowTextureSlot(normalTexture, "Normal", TextureType::Normal);
        ImGui::Text("Factor");
        ImGui::SameLine(70.0f);
        ImGui::SliderFloat("##Normal", &normal, 0.0f, 1.0f);

        ImGui::Separator();

        //Height
        ShowTextureSlot(heightTexture, "Height", TextureType::Height);
        ImGui::Text("Factor");
        ImGui::SameLine(70.0f);
        ImGui::SliderFloat("##Height", &height, 0.0f, 1.0f);

        ImGui::Separator();

        //Occlusion
        ShowTextureSlot(occlusionTexture, "Occlusion", TextureType::Occlusion);

        ImGui::Separator();

        //Emissive
        ShowTextureSlot(emissiveTexture, "Emissive", TextureType::Emissive);

        ImGui::Separator();

        //Mask
        ShowTextureSlot(maskTexture, "Mask", TextureType::Mask);

        ImGui::Separator();

        //Tiling
        ImGui::PushItemWidth(100.0f);
        ImGui::TextUnformatted("Tilling");
        ImGui::SameLine(70.0f);
        ImGui::TextUnformatted("X");
        ImGui::SameLine();
        ImGui::InputFloat("##TillingX", &tiling.x, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_CharsDecimal);
        ImGui::SameLine();
        ImGui::TextUnformatted("Y");
        ImGui::SameLine();
        ImGui::InputFloat("##TillingY", &tiling.y, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_CharsDecimal);
        ImGui::PopItemWidth();

        //Offset
        ImGui::PushItemWidth(100.0f);
        ImGui::TextUnformatted("Offset");
        ImGui::SameLine(70.0f);
        ImGui::TextUnformatted("X");
        ImGui::SameLine();
        ImGui::InputFloat("##OffsetX", &offset.x, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_CharsDecimal);
        ImGui::SameLine();
        ImGui::TextUnformatted("Y");
        ImGui::SameLine();
        ImGui::InputFloat("##OffsetY", &offset.y, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_CharsDecimal);
        ImGui::PopItemWidth();

        if (roughness != material->GetRoughnessMultiplier())    material->SetRoughnessMultiplier(roughness);
        if (metallic != material->GetMetallicMultiplier())      material->SetMetallicMultiplier(metallic);
        if (normal != material->GetNormalMultiplier())          material->SetNormalMultiplier(normal);
        if (height != material->GetHeightMultiplier())          material->SetHeightMultiplier(height);
        if (tiling != material->GetTiling())                    material->SetTiling(tiling);
        if (offset != material->GetOffset())                    material->SetOffset(offset);
        if (colorAlbedo != material->GetColorAlbedo())          material->SetColorAlbedo(colorAlbedo);
    }
}

void Widget_Inspector::ShowAddComponentButton()
{
    ImGui::Separator();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.5f - 50.0f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
    if (ImGui::Button("Add Component"))
        ImGui::OpenPopup("ComponentPopup");
}

void Widget_Inspector::ShowComponentPopup()
{
    if (auto actor = EditorHelper::Get().GetSelectActor())
    {
        if (ImGui::BeginPopup("ComponentPopup"))
        {
            if (ImGui::MenuItem("Camera"))
                actor->AddComponent<Camera>();
            if (ImGui::MenuItem("Renderer"))
                actor->AddComponent<Renderable>();

            ImGui::EndPopup();
        }
    }
}
