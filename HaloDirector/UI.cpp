#include "UI.h"
#include "Log.h"
#include "StdInc.h"

int ui_index = 0;



UI_Item ui_items_fake[UI_COUNT];
UI_Item ui_items[UI_COUNT];

int UI::GetIndex()
{
	return ui_index;
}

void UI::Execute(UI_CONTROLS type)
{
	ui_items[type].action();
}

void UI::Do()
{
	Execute((UI_CONTROLS)ui_index);
}

const char* UI::GetName(UI_CONTROLS type)
{
	return ui_items[type].name;
}

const char* UI::GetName(int type)
{
	return UI::GetName((UI_CONTROLS)type);
}

void UI::SetText(int type, const char* text)
{
	ui_items[type].last_error_time = 0;
	ui_items[type].name = text;
}

void UI::SetTextBackground(int type, const char* text)
{
	ui_items[type].name = text;
}

const char* UI::GetCurrentName()
{
	return ui_items[ui_index].name;
}

const char* UI::GetErrorText(int type)
{
	return ui_items[type].error_text;
}

void command()
{
	UI::Error(ui_index, "Error: Not Yet Implemented");
}

void timescale_func() {
	if (!Hooks::Initialised() || IsBadWritePtr(Halo::p_timescale, sizeof(float))) return;

	*Halo::p_timescale += 0.1;
	if (*Halo::p_timescale > 1.1)
		*Halo::p_timescale = 0.1;
}

void UI::Init()
{
	//UI_Item timescale{ "Timescale", &command };
	Log::Info("Initialising UI");

	ui_items[UI_TIMESCALE] = UI_Item{"Timescale", &timescale_func};
	ui_items[UI_GAMETICK] = UI_Item{"GameTick", &timescale_func};
	ui_items[UI_DOLLYTICK] = UI_Item{"DollyTick", &timescale_func};
	ui_items[UI_BEGIN_TIME] = UI_Item{"BeginTime", &timescale_func};
	ui_items[UI_SKIP_NEXT_MARKER] = UI_Item{"Skip To Next Marker", &DollyCam::SkipToNextMarker};
	ui_items[UI_BACK_LAST_MARKER] = UI_Item{"Back To Last Marker", &DollyCam::BackToLastMarker};
	ui_items[UI_PLAY_DOLLY] = UI_Item{"Play Dolly", &DollyCam::Play};
	ui_items[UI_RESTART] = UI_Item{"Restart Dolly", &DollyCam::Restart};
	ui_items[UI_EDIT_MARKER] = UI_Item{"Edit Marker", &DollyCam::EditClosestMarker};
	ui_items[UI_CREATE_MARKER_GAME] = UI_Item{"Create Marker(Game)", &DollyCam::AddMarkerGameTick};
	ui_items[UI_CREATE_MARKER_DOLLY] = UI_Item{"Create Marker(Dolly)", &DollyCam::AddMarkerDollyTick};
	ui_items[UI_DELETE_MARKER] = UI_Item{"Delete Marker", &DollyCam::RemoveClosestNode};
	ui_items[UI_DELETE_ALL] = UI_Item{"Delete All Markers", &DollyCam::RemoveAllNode};
}

void UI::Left()
{
	if (ui_index - 1 == -1)
	{
		ui_index = UI_COUNT - 1;
		return;
	}
	ui_index--;
}

void UI::Right()
{
	if (ui_index + 1 == UI_COUNT)
	{
		ui_index = 0;
		return;
	}
	ui_index++;
}

void UI::Error(int type, const char* text)
{
	ui_items[type].error_text = text;
	ui_items[type].last_error_time = GetTickCount64();
}

bool UI::DisplayError(int type)
{
	ULONGLONG now = GetTickCount64();

	if (now - ui_items[type].last_error_time > 5000)
		return false;
	return true;
}


