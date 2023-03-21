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
	void MainFunction();
	bool Update(Camera* p_Cam, float* fov);
	void AddMarker();
	void SkipToNextdMarker();
	void EditClosestMarker();
	void SetMarker(CameraMarker* cameraMarker, long long time_tick);
	bool RemoveNode(CamNode *node);
	void RemoveAllNode();
	void RemoveClosestNode();
	bool BetweenMarkers();
	Vector3 GetPositionForCurrentTime();
	CamNode* GetClosestNode();
	CamNode* GetHeaderNode();
	CamNode* GetNodeByIndex(size_t index);
	bool Playing();
	void Play();
	void Restart();
	void Init(uintptr_t module, uintptr_t teb);
}