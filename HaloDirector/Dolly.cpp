#include "Dolly.h"
#include <chrono>
#include "UI.h"

namespace DollyCam
{
	CamNode* head = NULL, *tail = NULL;
	CamNode* current_node = NULL;
	bool bplay = false;
	bool bEditing = false, bEditReady = false;
	bool bSync = false;

	long long speed_tick = 60;
	long long current_tick_dolly = 0;
	long long begin_tick = 0;
	unsigned long* p_gameTickTime;
	unsigned long gameTickTime;

	static __int64* TEBAddress;
	static __int64 hModule;
	__int32 TlsIndex;

	long long GetBeginTime() { return begin_tick; }
	long long GetDollyTick() { return current_tick_dolly; }
	long long GetGameTick() { if (Hooks::Initialised() && mem::PatchAOB(&gameTickTime, p_gameTickTime, sizeof(unsigned long))) return gameTickTime; else return 0; }
	CamNode* GetHeadNode() { return head; }
	CamNode* GetTailNode() { return tail; }
	bool Playing() { return bplay; }
	bool Editing() { return bEditing; }
	bool IsSync() { return bSync; }

	void Init(uintptr_t module, uintptr_t teb)
	{
		hModule = module;
		TEBAddress = (__int64*)teb;
		TlsIndex = *(__int32*)(hModule + 0xA38F9C);
		p_gameTickTime = (unsigned long*)(*(__int64*)(*(__int64*)(TEBAddress + TlsIndex) + 0xC8i64) + 0xC);
		Halo::p_Cam = (Camera*)(*(__int64*)(*(__int64*)(TEBAddress + TlsIndex) + 0x188i64) + 0x8);
		Log::Debug("Camera Address:%llX Game Tick Address:%llX", Halo::p_Cam, p_gameTickTime);
		RemoveAllNode();
	}

	void Uninit()
	{
		p_gameTickTime = 0;
		Halo::p_Cam = 0;
		RemoveAllNode();
	}

	void MainFunction()
	{

		if (bplay)
		{
			if (Update(Halo::p_Cam,Halo::p_fov))
			{
				current_tick_dolly++;
			}
			else
			{
				bplay = false;
				if (bSync && Halo::p_timescale) *Halo::p_timescale = 0.0f;
			}
		}
	}
	//todo
	bool Update(Camera* p_Cam, float* fov)
	{
		CamNode* m0, * m1, * m2, * m3;
		CamNode* node = NULL;
		int i = 0;

		current_node = GetCurrentNode();

		if (current_node == NULL || p_Cam == nullptr || fov == nullptr || !Hooks::Initialised()) return false;

		node = m0 = m1 = m2 = m3 = current_node;
		if (current_node->prev != NULL) m0 = current_node->prev;
		while (i < 3)
		{
			if (i == 1) m2 = node;
			if (i == 2) m3 = node;
			if (node->next != NULL) node = node->next;
			i++;
		}

		//We gotta do it like this to break free from the game's horrible tick rate
		float current_time_relative = current_tick_dolly - m1->t->time_relative;
		float _tmp = m2->t->time_relative - m1->t->time_relative == 0 ? 1.0f : (float)m2->t->time_relative - m1->t->time_relative;
		float alpha = current_time_relative / _tmp;

		//lerp position
		p_Cam->position = Math::CatmullRomInterpolate(
			m0->t->position,
			m1->t->position,
			m2->t->position,
			m3->t->position,
			alpha,
			0.0f);

		//lerp forward position and then look at it. idk cod did it like this 
		Vector3 forward = Math::CatmullRomInterpolate(
			m0->t->forward,
			m1->t->forward,
			m2->t->forward,
			m3->t->forward,
			alpha,
			0.0f);

		p_Cam->rotation = Math::LookAt(p_Cam->position, forward);

		//we just gonna use cosine interpolation for these because Catmullrom was being fucky. ill come back to this
		p_Cam->rotation.z = Math::CosineInterpolate(m1->t->roll, m2->t->roll, alpha);
		*fov = Math::CosineInterpolate(m1->t->fov, m2->t->fov, alpha);

		// reach end
		if (current_node->next == NULL) return false;
		else return true;
	}

	void UpdateDollyTime()
	{
		if (head == NULL) return;
		long long offset = head->t->time_relative;
		if (offset == 0) return;
		CamNode* node = head;

		while (node != NULL)
		{
			node->t->time_relative -= offset;
			node = node->next;
		}
		current_tick_dolly -= offset;
		begin_tick += offset;
	}

	void AddDollyTick(long long tick)
	{
		current_tick_dolly += tick;
		Update(Halo::p_Cam, Halo::p_fov);
	}

	void AddMarker(long long tick)
	{
		if (bplay || !Hooks::Initialised()|| tick < 0) return;

		CameraMarker* _new_marker_ = new CameraMarker;
		CamNode* _new_node_ = new CamNode;
		long long time_tick_relative = tick - begin_tick;

		memset(_new_marker_, 0, sizeof(CameraMarker));
		memset(_new_node_, 0, sizeof(CamNode));
		_new_node_->t = _new_marker_;

		//New Chain
		if (head == NULL)
		{
			begin_tick = GetGameTick();
			SetMarker(_new_marker_, tick);
			head = tail = _new_node_;
			Log::Debug("Head:%llX\n", head);
			return;
		}

		if (time_tick_relative < head->t->time_relative)
		{
			SetMarker(_new_marker_, tick);
			_new_node_->next = head;
			head = _new_node_;
			UpdateDollyTime();
			return;
		}

		//Insert Node Into Chain
		CamNode* node = head;
		while (node->next != NULL && node->next->t->time_relative <= time_tick_relative)
		{
			node = node->next;
		}
		_new_node_->next = node->next;
		node->next = _new_node_;
		_new_node_->prev = node;
		if (_new_node_->next == NULL)	tail = _new_node_;
		SetMarker(_new_marker_, tick);
	}

	void AddMarkerGameTick()
	{
		if (bplay || !Hooks::Initialised()) return;
		AddMarker(*p_gameTickTime);
	}

	void AddMarkerDollyTick()
	{
		if (bplay || !Hooks::Initialised()) return;
		if (head == NULL)
			AddMarkerGameTick();
		else
			AddMarker(begin_tick + current_tick_dolly);
	}

	bool RemoveNode(CamNode* node)
	{
		bEditing = true;
		if (node == NULL) return false;

		if (node->prev != NULL) node->prev->next = node->next;
		else head = head->next;
		if (node->next != NULL) node->next->prev = node->prev;
		else tail = tail->prev;
		delete node->t;
		delete node;
		UpdateDollyTime();
		bEditing = false;
		return true;
	}

	void RemoveAllNode()
	{
		bEditing = true;

		bplay = false;
		CamNode* node = head;
		CamNode* next = NULL;
		while (node != NULL)
		{
			next = node->next;
			delete node->t;
			delete node;
			node = next;
		}
		head = NULL;
		begin_tick = 0;
		current_tick_dolly = 0;
		bEditReady = false;
		gameTickTime = 0;

		bEditing = false;
	}

	void RemoveClosestNode()
	{
		if (!Hooks::Initialised()) return;
		bplay = false;
		CamNode* node = GetClosestNode();
		if (node == NULL)
		{
			Log::Info("Failed To Find Closest Node");
			UI::Error(UI_DELETE_MARKER, "No Marker Selected");
			return;
		}
		RemoveNode(node);
	}

	bool BetweenMarkers()
	{
		CamNode* node = head;

		if (bEditing) return false;

		while (node != NULL)
		{
			if (node->next != NULL)
				if (node->t->time_relative < current_tick_dolly && node->next->t->time_relative > current_tick_dolly)
					return true;
			node = node->next;
		}
		return false;
	}

	Vector3 GetPositionForCurrentTime()
	{
		CamNode* m0, * m1, * m2, * m3;
		CamNode* node = NULL;
		int i = 0;

		current_node = GetCurrentNode();

		if (current_node == NULL || current_node->next == NULL) return Vector3();

		node = m0 = m1 = m2 = m3 = current_node;
		if (current_node->prev != NULL) m0 = current_node->prev;
		while (i < 3)
		{
			if (i == 1) m2 = node;
			if (i == 2) m3 = node;
			if (node->next != NULL) node = node->next;
			i++;
		}

		//We gotta do it like this to break free from the game's horrible tick rate
		float current_time_relative = current_tick_dolly - current_node->t->time_relative;
		//float alpha = current_time_relative / (Dolly::markers[i + 1].time_relative - Dolly::markers[i].time_relative);

		float alpha = current_time_relative / (current_node->next->t->time_relative - current_node->t->time_relative);

		//lerp position
		return Math::CatmullRomInterpolate(
			m0->t->position,
			m1->t->position,
			m2->t->position,
			m3->t->position,
			alpha,
			0.0f);
	}

	CamNode* GetClosestNode()
	{
		float dist = 1e27f;
		CamNode *node, *result = NULL;

		node = head;
		while (node != NULL)
		{
			float distance = Math::WorldToScreen(node->t->position, 1920, 1080).z;
			if (distance < dist)
			{
				dist = distance;
				result = node;
			}
			node = node->next;
		}

		Log::Info("Closest Distance: %f", dist);

		//only return an index if closer than 0.5
		if (dist >= 0.5)
			result = NULL;
		return result;
	}

	CamNode* GetCurrentNode()
	{
		CamNode* node, *current_node = NULL;

		if (bEditing) return NULL;

		if (current_tick_dolly == 0)// beginning
		{
			current_node = head;
		}
		else if (head == NULL || current_tick_dolly > tail->t->time_relative || current_tick_dolly < 0)
		{
			current_node = NULL;
		}
		else
		{
			node = head;
			while (node != NULL)
			{
				if (node->t->time_relative <= current_tick_dolly)
					current_node = node;
				node = node->next;
			}
		}
		return current_node;
	}

	CamNode* GetNodeByIndex(size_t index)
	{
		size_t i = 0;
		CamNode* node = head;

		while (node != NULL)
		{
			if (i == index) return node;
			node = node->next;
			i++;
		}
		// Fail to find node(index)
		return NULL;
	}

	CamNode* EditNode;

	void EditClosestMarker()
	{
		if (!Hooks::Initialised()) return;
		if (!bEditReady)
		{
			EditNode = GetClosestNode();
			if (EditNode == NULL) {
				UI::Error(UI_EDIT_MARKER, "No Marker Selected");
				return;
			}
			current_tick_dolly = EditNode->t->time_relative;
			bEditReady = true;
			UI::SetText(UI_EDIT_MARKER, "Editing Marker");
		}
		else
		{
			UI::SetText(UI_EDIT_MARKER, "Edit Marker");
			RemoveNode(EditNode);
			AddMarkerDollyTick();
			bEditReady = false;
		}
	}
	void SetMarker(CameraMarker* cameraMarker, long long time_tick)
	{
		if (Halo::p_Cam == NULL || IsBadWritePtr(Halo::p_Cam,sizeof(Camera))) return;
		Camera c = *Halo::p_Cam;

		cameraMarker->time_relative = time_tick - begin_tick;
		cameraMarker->position = c.position;
		cameraMarker->forward = Math::GetForwardPosition(0.5f, c.position, c.rotation);
		cameraMarker->fov = *Halo::p_fov;
		cameraMarker->roll = c.rotation.z;
	}

	void SkipToNextMarker()
	{
		if (!Hooks::Initialised()) return;
		current_node = GetCurrentNode();
		if (current_node == NULL || current_node->next == NULL) // tail
			current_tick_dolly = 0;
		else
			current_tick_dolly = current_node->next->t->time_relative;
		Update(Halo::p_Cam, Halo::p_fov);
	}

	void BackToLastMarker()
	{
		if (!Hooks::Initialised()) return;
		current_node = GetCurrentNode();
		if (current_node == NULL)
			current_tick_dolly = 0;
		else if (current_node->prev == NULL) // head
			current_tick_dolly = tail->t->time_relative;
		else
			current_tick_dolly = current_node->prev->t->time_relative;
		Update(Halo::p_Cam, Halo::p_fov);
	}

	void Play()
	{
		if (!Hooks::Initialised()) return;
		if (current_node == NULL || current_node == tail || current_tick_dolly < 0)
		{
			Restart();
		}
		bplay = !bplay;
		if (bSync && Halo::p_timescale) *Halo::p_timescale = bplay ? 1.0f : 0.0f;
	}
	void Restart()
	{
		if (!Hooks::Initialised()) return;
		current_node = head;
		bplay = false;
		if (bSync && Halo::p_timescale) *Halo::p_timescale = 0.0f;
		current_tick_dolly = 0;
		Update(Halo::p_Cam, Halo::p_fov);
	}

	namespace Console
	{
		void Save(const char* arg)
		{
			FILE* file = NULL;
			char name[MAX_PATH];
			std::string path(MAX_PATH, '0');

			if (arg == NULL)
			{
				Log::Info("Received 0 Argument. Expected 1 Argument.");
				return;
			}

			CreateDirectoryA("Dolly", NULL);

			GetCurrentDirectoryA(MAX_PATH, name);
			sprintf_s((LPSTR)path.c_str(), MAX_PATH, "%s\\Dolly\\%s.dolly",name, arg);

			if (fopen_s(&file, path.c_str(), "wb"))
			{
				Log::Info("Open File:%s Failed!", arg);
				return;
			}

			CamNode* node = head;
			while (node != NULL)
			{
				fwrite(node->t, sizeof(CameraMarker), 1, file);
				node = node->next;
			}

			Log::Info("Save Success!\nPath:%s", path.c_str());
			fclose(file);
		}

		void Load(const char* arg)
		{
			FILE* file = NULL;
			char name[MAX_PATH];
			std::string path(MAX_PATH, '0');

			if (arg == NULL)
			{
				Log::Info("Received 0 Argument. Expected 1 Argument.");
				return;
			}

			CreateDirectoryA("Dolly", NULL);

			GetCurrentDirectoryA(MAX_PATH, name);
			sprintf_s((LPSTR)path.c_str(), MAX_PATH, "%s\\Dolly\\%s.dolly", name, arg);

			if (fopen_s(&file, path.c_str(), "rb"))
			{
				Log::Info("Open File:%s Failed!", arg);
				return;
			}

			RemoveAllNode();

			CamNode* _new_node_;
			CameraMarker marker, *_new_marker;

			while (fread(&marker, sizeof(CameraMarker), 1, file))
			{
				_new_node_ = new CamNode;
				_new_marker = new CameraMarker;

				memset(_new_node_, 0, sizeof(CamNode));
				_new_node_->t = _new_marker;

				memcpy(_new_marker, &marker, sizeof(CameraMarker));

				if (head == NULL)
				{
					head = tail = _new_node_;
					continue;
				}

				CamNode* _node = head;
				while (_node->next != NULL)	_node = _node->next;
				_node->next = _new_node_;
				_new_node_->prev = _node;
				tail = _new_node_;
			}

			Log::Info("Load Success!\nPath:%s", path.c_str());
			fclose(file);
		}

		void SetBeginTime(const char* arg)
		{
			if (arg == NULL)
			{
				Log::Info("Received 0 Argument. Expected 1 Argument.");
				return;
			}

			long long num = 0;

			try {
				num = std::stoll(arg);
			}
			catch (std::invalid_argument const& e) {
				Log::Error("Console Commands -> Invalid Argument");
			}
			catch (std::out_of_range const& e) {
				Log::Error("Console Commands -> Out of Range");
			}

			begin_tick = num;
		}

		void SetDollyTick(const char* arg)
		{
			if (arg == NULL)
			{
				Log::Info("Received 0 Argument. Expected 1 Argument.");
				return;
			}

			long long num = 0;

			try {
				num = std::stoll(arg);
			}
			catch (std::invalid_argument const& e) {
				Log::Error("Console Commands -> Invalid Argument");
			}
			catch (std::out_of_range const& e) {
				Log::Error("Console Commands -> Out of Range");
			}

			current_tick_dolly = num;
		}

		void SetDollySync(const char* arg)
		{
			if (arg == NULL)
			{
				Log::Info("Received 0 Argument. Expected 1 Argument.");
				return;
			}

			long long num = 0;

			try {
				num = std::stoll(arg);
			}
			catch (std::invalid_argument const& e) {
				Log::Error("Console Commands -> Invalid Argument");
			}
			catch (std::out_of_range const& e) {
				Log::Error("Console Commands -> Out of Range");
			}

			bSync = num;
		}

		void Init()
		{
			ConsoleCommands::Add("dolly_save_path", &Save);
			ConsoleCommands::Add("dolly_load_path", &Load);
			ConsoleCommands::Add("dolly_set_begin", &SetBeginTime);
			ConsoleCommands::Add("dolly_set_tick", &SetDollyTick);
			ConsoleCommands::Add("dolly_set_sync", &SetDollySync);
		}
	}
}