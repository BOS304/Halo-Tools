#pragma once
#include <string>
#include <Windows.h>
enum UI_CONTROLS {
	UI_TIMESCALE,
	UI_GAMETICK,
	UI_SKIP_NEXT_MARKER,
	UI_BACK_LAST_MARKER,
	UI_PLAY_DOLLY,
	UI_RESTART,
	UI_EDIT_MARKER,
	UI_CREATE_MARKER_GAME,
	UI_CREATE_MARKER_DOLLY,
	UI_DELETE_MARKER,
	UI_DELETE_ALL,
	UI_BEGIN_TIME,
	UI_DOLLYTICK,
	UI_COUNT
};

typedef void(UI_Function)();

struct UI_Item {
	const char* name = "";
	UI_Function* action;

	const char* error_text = "";
	ULONGLONG last_error_time;
};

class UI
{
public:
	static int GetIndex();

	static void Execute(UI_CONTROLS type);

	static void Do();

	static const char* GetName(UI_CONTROLS type);

	static const char* GetName(int type);

	static void SetText(int type, const char* text);

	static void SetTextBackground(int type, const char* text);

	static const char* GetCurrentName();

	static const char* GetErrorText(int type);

	static void Init();

	static void Left();

	static void Right();

	static void Error(int type, const char* text);

	static bool DisplayError(int type);
};
