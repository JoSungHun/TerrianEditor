#include "stdafx.h"
#include "Widget_Log.h"

Widget_Log::Widget_Log(Context * context)
	: IWidget(context)
	, maxLogCount(999)
	, bShowInfo(true)
	, bShowWarning(true)
	, bShowError(true)
{
	title = "Log";

	logger = new EngineLogger();
	logger->SetCallBack([this](const LogPair& logPair) { AddLog(logPair); });

	Log::SetLogger(logger);

	Log::Write("dddddddddddddddddddddd", LogType::Warning);
	Log::Write("dddddddddddddddddddddd", LogType::Warning);
	Log::Write("dddddddddddddddddddddd", LogType::Warning);
	Log::Write("dddddddddddddddddddddd", LogType::Warning);
	Log::Write("dddddddddddddddddddddd", LogType::Warning);
	Log::Write("dddddddddddddddddddddd", LogType::Warning);
	Log::Write("dddddddddddddddddddddd", LogType::Warning);

	Log::Write("dddddddddddddddddddddd", LogType::Info);
	Log::Write("dddddddddddddddddddddd", LogType::Info);
	Log::Write("dddddddddddddddddddddd", LogType::Info);
	Log::Write("dddddddddddddddddddddd", LogType::Info);
	Log::Write("dddddddddddddddddddddd", LogType::Info);
	Log::Write("dddddddddddddddddddddd", LogType::Info);
	Log::Write("dddddddddddddddddddddd", LogType::Info);

	Log::Write("dddddddddddddddddddddd", LogType::Error);
	Log::Write("dddddddddddddddddddddd", LogType::Error);
	Log::Write("dddddddddddddddddddddd", LogType::Error);
	Log::Write("dddddddddddddddddddddd", LogType::Error);
	Log::Write("dddddddddddddddddddddd", LogType::Error);
	Log::Write("dddddddddddddddddddddd", LogType::Error);
	Log::Write("dddddddddddddddddddddd", LogType::Error);

}

Widget_Log::~Widget_Log()
{
	Clear();
}

void Widget_Log::Render()
{
	static ImGuiTextFilter logFilter;
	static std::vector<ImVec4> logColor =
	{
		ImVec4(0.76f, 0.77f, 0.8f, 1.0f), // Info
		ImVec4(0.75f, 0.75f, 0.0f, 1.0f), // Warning
		ImVec4(0.75f, 0.0f,  0.0f, 1.0f), // Error
	};

	auto DisplayButton = [this](const IconType& type, bool* toggle)
	{
		ImGui::PushStyleColor
		(
			ImGuiCol_Button,
			ImGui::GetStyle().Colors[*toggle ? ImGuiCol_ButtonActive : ImGuiCol_Button]
		);

		if (IconProvider::Get().ImageButton(type, 15.0f))
		{
			*toggle = !(*toggle);
			bScroll = true;
		}

		ImGui::PopStyleColor();
		ImGui::SameLine();
	};

	if (ImGui::Button("Clear"))
		Clear();

	ImGui::SameLine();

	DisplayButton(IconType::Log_Info, &bShowInfo);
	DisplayButton(IconType::Log_Warning, &bShowWarning);
	DisplayButton(IconType::Log_Error, &bShowError);

	logFilter.Draw("Filter", -100.0f);
	ImGui::Separator();

	ImGui::BeginChild("Logging", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
	{
		for (const auto& log : logs)
		{
			if (!logFilter.PassFilter(log.text.c_str()))
				continue;

			bool bCheck = false;
			bCheck |= log.type == 0 && bShowInfo;
			bCheck |= log.type == 1 && bShowWarning;
			bCheck |= log.type == 2 && bShowError;

			if (bCheck)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, logColor[log.type]);
				ImGui::TextUnformatted(log.text.c_str());
				ImGui::PopStyleColor();
			}
		}

		if (bScroll)
		{
			ImGui::SetScrollHereY();
			bScroll = false;
		}
	}
	ImGui::EndChild();
}

void Widget_Log::AddLog(const LogPair & logPair)
{
	logs.push_back(logPair);

	if (logs.size() > maxLogCount)
		logs.pop_front();

	bScroll = true;
}

void Widget_Log::Clear()
{
	logs.clear();
	logs.shrink_to_fit();
}
