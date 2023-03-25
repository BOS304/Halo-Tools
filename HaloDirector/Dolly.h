#pragma once
#include "StdInc.h"

#define MAX_DOLLY_MARKERS 50

namespace DollyCam
{
	struct CamNode
	{
		CameraMarker *t;
		CamNode* next;
		CamNode* prev;
	};

	long long GetDollyTick();
	long long GetGameTick();
	long long GetBeginTime();
	void MainFunction();
	bool Update(Camera* p_Cam, float* fov);
	void UpdateDollyTime();
	void AddDollyTick(long long tick);
	void AddMarkerGameTick();
	void AddMarkerDollyTick();
	void SkipToNextMarker();
	void BackToLastMarker();
	void EditClosestMarker();
	void SetMarker(CameraMarker* cameraMarker, long long time_tick);
	bool RemoveNode(CamNode *node);
	void RemoveAllNode();
	void RemoveClosestNode();
	bool BetweenMarkers();
	Vector3 GetPositionForCurrentTime();
	CamNode* GetClosestNode();
	CamNode* GetCurrentNode();
	CamNode* GetHeadNode();
	CamNode* GetTailNode();
	CamNode* GetNodeByIndex(size_t index);
	bool Playing();
	bool Editing();
	void Play();
	void Restart();
	void Init(uintptr_t module, uintptr_t teb);
	namespace Console { void Init(); }
}