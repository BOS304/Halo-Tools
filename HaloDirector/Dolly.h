#pragma once
#include "StdInc.h"

#define MAX_DOLLY_MARKERS 50

static class Dolly
{
public:
	static CameraMarker markers[MAX_DOLLY_MARKERS];
	static unsigned long long time;
	static unsigned long tick;
	static int count;
	static void play();
	static void ToggleDolly();
	static bool IsDollying();
	static void update(int arg);
	static void addMarker();
	static void Pause();
	static void Resume();
	static void removeMarker(int index);
	static void removeSelected();
	static void removeAll();
	static void Initialise();
	static bool DollyExistsAtCurrentTime();
	static int GetNextMarkerInTimeline();
	static void SkipToNextMarker();
	static void SkipToMarker(int index);
	static DWORD WINAPI Loop(LPVOID Param);
	static Vector3 GetPositionForCurrentTime();
	static bool BetweenMarkers();
	static int GetSelectedMarkerIndex();
	static void EditMarker();
};

namespace DollyCam
{
	struct CamNode
	{
		CameraMarker *t;
		CamNode* next;
		CamNode* prev;
	};
	void MainFunction();
	bool Update(Camera* Cam, float* fov);
	void AddMarker();
	void SetMarker(CameraMarker* cameraMarker, long long time_tick);
	bool RemoveNode(CamNode *node);
	bool RemoveClosestNode();
	bool BetweenMarkers();
	Vector3 GetPositionForCurrentTime();
	CamNode* GetClosestNode();
	CamNode* GetHeaderNode();
	CamNode* GetNodeByIndex(size_t index);
	bool Playing();
	void Play();
	void Pause();
	void Restart();
	void Init(uintptr_t module, uintptr_t teb);
}