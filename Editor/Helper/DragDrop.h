#pragma once
#include "stdafx.h"

enum class DragDropPayloadType : uint
{
    Unknown,
    Texture,
    Actor,
    Audio,
    Script,
};

class DragDrop final
{
public:
    static void SetDragDrop(const DragDropPayloadType& type, const std::string& data)
    {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload
            (
                reinterpret_cast<const char*>(&type),
                data.c_str(),
                data.length() + 1
            );
            ImGui::TextUnformatted(data.c_str());
            ImGui::EndDragDropSource();
        }
    }

    static auto GetDragDrop(const DragDropPayloadType& type) -> const std::string 
    {
        if (ImGui::BeginDragDropTarget())
        {
            const auto payload = ImGui::AcceptDragDropPayload
            (
                reinterpret_cast<const char*>(&type)
            );

            if (payload)
                return reinterpret_cast<const char*>(payload->Data);

            ImGui::EndDragDropTarget();
        }
        return "";
    }
};