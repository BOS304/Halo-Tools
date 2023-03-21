#include "Dolly.h"
#include <chrono>
#include "UI.h"
using namespace Halo;

float start_time_real = 0.0f;
float start_time = 0.0f;
CameraMarker Dolly::markers[MAX_DOLLY_MARKERS];
int Dolly::count = 0;

unsigned long Dolly::tick = 60;
unsigned long long Dolly::time = 0;

bool doDolly = false;
bool dollyStarted = false;
float lastTime = 0;
float alpha = 0.0f;


//float GetCurrentSeconds() {
//	auto uptime = GetTickCount64();
//	double seconds = (double)uptime / 1000.0;
//	return float(seconds);
//}


void Dolly::play()
{
	if (count >= 2)
	{
		doDolly = true;

		UI::SetText(UI_PLAY_DOLLY, "Stop Dolly");
	}
	else {
		UI::Error(UI_PLAY_DOLLY, "Error: Require atleast 2 markers for dolly");
	}
}

void InitDolly() {

	start_time = *serverTime;

	Dolly::time = 0;

	Log::Info("Beginning Dolly Cam");
}

void demo_stop_dolly(int arg)
{
	dollyStarted = false;
	doDolly = false;
	UI::SetText(UI_PLAY_DOLLY, "Play Dolly");
}

void Dolly::ToggleDolly()
{
	if (doDolly == false)
	{
		Dolly::play();
	}
	else {
		demo_stop_dolly(0);
	}
}

bool Dolly::IsDollying()
{
	return doDolly;
}

bool afterLastMarker()
{
	if (*serverTime > Dolly::markers[Dolly::count - 1].time)
		return true;
	return false;
}

void Dolly::update(int arg)
{
	for (int i = 0; i < Dolly::count; i++)
	{
		//if between two markers:
		if (time > Dolly::markers[i].time && time < Dolly::markers[i + 1].time)
		{
			CameraMarker* m0;
			CameraMarker* m1;
			CameraMarker* m2;
			CameraMarker* m3;

			//these are just for debugging
			int i0, i1, i2, i3;

			//surely theres a better way to do this
			//however, im stupid and this works.
			//i got that cave man brain
			if (i > 0)
			{
				m0 = &Dolly::markers[i - 1];
				i0 = i - 1;

				m1 = &Dolly::markers[i];
				i1 = i;
			}
			else {
				m0 = &Dolly::markers[i];
				m1 = &Dolly::markers[i];

				i0 = i;
				i1 = i;
			}
			
			if ((Dolly::count - i) > 2)
			{
				m2 = &Dolly::markers[i + 1];
				i2 = i + 1;

				m3 = &Dolly::markers[i + 2];
				i3 = i + 2;
			}
			else {
				m2 = &Dolly::markers[i + 1];
				i2 = i + 1;
				i3 = i + 1;

				m3 = &Dolly::markers[i + 1];
			}

			//We gotta do it like this to break free from the game's horrible tick rate
			float current_time_relative = *serverSeconds - start_time_real - Dolly::markers[i].time_relative;
			float alpha = current_time_relative / (Dolly::markers[i + 1].time_relative - Dolly::markers[i].time_relative);

			//Log::Info("%d] Count: %d Indices: %d, %d, %d, %d,      -     %f",i, Dolly::count, i0, i1, i2, i3, alpha);

			//lerp position
			Cam->position = Math::CatmullRomInterpolate(
				m0->position, 
				m1->position,
				m2->position,
				m3->position,
				alpha, 
				0.0f);

			//lerp forward position and then look at it. idk cod did it like this 
			Vector3 forward = Math::CatmullRomInterpolate(
				m0->forward,
				m1->forward,
				m2->forward,
				m3->forward,
				alpha,
				0.0f);

			Cam->rotation = Math::LookAt(Cam->position, forward);

			//we just gonna use cosine interpolation for these because Catmullrom was being fucky. ill come back to this
			Cam->rotation.z =	Math::CosineInterpolate(m1->roll, m2->roll, alpha);
			*fov =									Math::CosineInterpolate(m1->fov, m2->fov, alpha);
			break;
		}
		time += tick;
	}
}

int getIndexFromTime() {
	for (int i = 0; i < Dolly::count; i++)
	{
		if (*serverTime < Dolly::markers[i].time)
		{
			return i;
		}
	}
	return Dolly::count;
}

void insert_marker(int index, CameraMarker marker)
{

	//Log::Debug("%s, %f", UI::GetName(0), 5.1);
	//shift elements forward
	for (int i = MAX_DOLLY_MARKERS-1; i >= index; i--)
	{
		Dolly::markers[i] = Dolly::markers[i - 1];
		Log::Debug("%s, %f", UI::GetName(0), 5.2);
	}

	//Log::Debug("%s, %f", UI::GetName(0), 5.3);

	Dolly::markers[index] = marker;
	//Log::Debug("%s, %f", UI::GetName(0), 5.4);
	Dolly::count++;
}

void Dolly::addMarker()
{
	if (!DollyExistsAtCurrentTime())
	{
		if (Dolly::count < MAX_DOLLY_MARKERS)
		{
			int index = getIndexFromTime();

			Camera c = *Cam;

			Log::Debug("Adding Dolly Marker");
			CameraMarker marker = CameraMarker();

			marker.time = *serverTime;
			marker.position = c.position;
			marker.forward = Math::GetForwardPosition(0.5f, c.position, c.rotation);
			marker.fov = *fov;
			marker.roll = c.rotation.z;
			insert_marker(index, marker);
		}
		else {
			//MessageBox(NULL, L"Maximum number of dolly markers reached", L"Error", NULL);
			UI::Error(UI_CREATE_MARKER, "Error: Maximum number of dolly markers reached");
		}
	}
	else {
		UI::Error(UI_CREATE_MARKER, "Error: A camera marker already exists at the current time.");
		//MessageBox(0, L"A Camera Marker already exists at the current time.", L"Error:", NULL);
	}
}

void Dolly::Pause()
{

}

void Dolly::Resume()
{
}

bool Dolly::BetweenMarkers() {
	for (int i = 0; i < Dolly::count; i++)
	{
		if (*serverTime > Dolly::markers[i].time && *serverTime < Dolly::markers[i + 1].time)
		{
			return true;
		}
	}
	return false;
}

//Find nearest marker
int Dolly::GetSelectedMarkerIndex()
{
	float dist = 9999999.0f;
	int index = -1;
	for (int i = 0; i < count; i++)
	{
		//float distance = Math::distance(Cam->position, markers[i].position);

		float distance = Math::WorldToScreen(markers[i].position, 1920, 1080).z;

		Log::Info("Distance: %f", distance);

		if (distance < dist)
		{
			dist = distance;
			index = i;
		}
	}

	Log::Info("Closest Distance: %f", dist);

	//only return an index if closer than 0.5
	if(dist < 0.5)
		return index;
	return -1;
}


bool editing = false;
int editing_index = -1;
void Dolly::EditMarker()
{
	Log::Info("Editing Marker!");
	//if not currently editing, skip to marker and copy camera location
	if (editing == false)
	{
		int index = GetSelectedMarkerIndex();
		if (index != -1) {
			editing_index = index;
		}
		else
		{
			UI::Error(UI_EDIT_MARKER, "No Marker Selected");
			return;
		}

		editing = true;

		UI::SetText(UI_EDIT_MARKER, "Skipping forward to marker");

		CameraMarker* selected = &markers[editing_index];

		if (selected->time > * serverTime)
			SkipToMarker(editing_index);

		Cam->position = selected->position;
		Cam->rotation = Math::LookAt(Cam->position, selected->forward);
		*fov = selected->fov;

		UI::SetText(UI_EDIT_MARKER, "Editing Marker");
		
	}
	//else if was editing, apply new camera info to that marker
	else {
		if (editing_index != -1)
		{
			CameraMarker* selected = &markers[editing_index];

			selected->position = Cam->position;
			selected->forward = Math::GetForwardPosition(1.5f, Cam->position, Cam->rotation);
			selected->fov = *fov;

			editing = false;
			editing_index = -1;
			UI::SetTextBackground(UI_EDIT_MARKER, "Edit Marker");
			UI::Error(UI_EDIT_MARKER, "Successfully Edited Marker");
		}
	}
}

Vector3 Dolly::GetPositionForCurrentTime() {
	for (int i = 0; i < Dolly::count; i++)
	{
		//if between two markers:
		if (*serverTime > Dolly::markers[i].time && *serverTime < Dolly::markers[i + 1].time)
		{
			CameraMarker* m0;
			CameraMarker* m1;
			CameraMarker* m2;
			CameraMarker* m3;

			//these are just for debugging
			int i0, i1, i2, i3;

			//surely theres a better way to do this
			//however, im stupid and this works.
			//i got that cave man brain
			if (i > 0)
			{
				m0 = &Dolly::markers[i - 1];
				i0 = i - 1;

				m1 = &Dolly::markers[i];
				i1 = i;
			}
			else {
				m0 = &Dolly::markers[i];
				m1 = &Dolly::markers[i];

				i0 = i;
				i1 = i;
			}

			if ((Dolly::count - i) > 2)
			{
				m2 = &Dolly::markers[i + 1];
				i2 = i + 1;

				m3 = &Dolly::markers[i + 2];
				i3 = i + 2;
			}
			else {
				m2 = &Dolly::markers[i + 1];
				i2 = i + 1;
				i3 = i + 1;

				m3 = &Dolly::markers[i + 1];
			}

			//We gotta do it like this to break free from the game's horrible tick rate
			float current_time_relative = *serverSeconds - start_time_real - Dolly::markers[i].time_relative;
			//float alpha = current_time_relative / (Dolly::markers[i + 1].time_relative - Dolly::markers[i].time_relative);

			float alpha = (*serverTime - Dolly::markers[i].time) / (Dolly::markers[i + 1].time - Dolly::markers[i].time);

			//lerp position
			return Math::CatmullRomInterpolate(
				m0->position,
				m1->position,
				m2->position,
				m3->position,
				alpha,
				0.0f);
		}
	}
}



void demo_begin_dolly(int arg)
{
	Dolly::play();
}

void demo_add_marker(int arg)
{
	Dolly::addMarker();
}

void demo_remove_marker(int arg)
{
	Dolly::removeMarker(arg);
}

void Dolly::removeMarker(int index)
{
	for (int i = index; i < count - 1; i++)
	{
		markers[i] = markers[i + 1];
	}
	count -= 1;
}



void Dolly::removeSelected()
{
	int index = GetSelectedMarkerIndex();
	if (index != -1)
		removeMarker(index);
	else
		UI::Error(UI_DELETE_MARKER, "No Marker Selected");
}

bool Dolly::DollyExistsAtCurrentTime() {
	for (int i = 0; i < MAX_DOLLY_MARKERS; i++)
	{
		if (Dolly::markers[i].time == *serverTime)
			return true;
	}
	return false;
}

int Dolly::GetNextMarkerInTimeline()
{
	for (int i = 0; i < count; i++)
	{
		if (*serverTime < markers[i].time)
			return i;
	}
	return -1;
}

void Dolly::SkipToNextMarker()
{
	int nextMarkerIndex = GetNextMarkerInTimeline();

	if (nextMarkerIndex != -1)
	{
		SkipToMarker(nextMarkerIndex);
	}
	else {
		UI::Error(UI_SKIP_NEXT_MARKER, "No Next Marker");
	}

}

void Dolly::SkipToMarker(int index)
{
	*timescale = 10;

	while (*serverTime < markers[index].time - 0.2f)
	{
		Sleep(1);
	}

	*timescale = 0;

	Log::Info("Server Time: %f", *serverTime);
	Log::Info("Marker Time: %f", markers[index].time);
}

void Dolly::removeAll()
{
	Dolly::count = 0;
	for (int i = 0; i < MAX_DOLLY_MARKERS; i++)
	{
		Dolly::markers[i] = CameraMarker();
	}
}

void demo_remove_all(int arg) {
	Dolly::removeAll();
}

void demo_test(int arg) {
	//(*world)->player->camera.position = Math::GetForwardPosition(0.5f, (*world)->player->camera.position, (*world)->player->camera.rotation);

	while (Cam->rotation.x > Math::radians(360))
		Cam->rotation.x -= Math::radians(360);

	while (Cam->rotation.x < -Math::radians(360))
		Cam->rotation.x += Math::radians(360);
}

Vector3 testPosition;

DWORD __stdcall Dolly::Loop(LPVOID Param)
{
	while (true) {
		Sleep(1);

		if (Cam) {
			if (*serverTime < lastTime)
			{
				float diff = lastTime - *serverTime;
				start_time_real += diff;
				Log::Info("Jump Backwards detected: Offsetting start time by: %f", diff);
			}

			if (doDolly)
			{
				if (Dolly::count > 1)
				{
					if (dollyStarted == false)
					{
						dollyStarted = true;
						InitDolly();
						Dolly::update(0);
					}
					else if (dollyStarted && *serverTime > Dolly::markers[0].time && *serverTime < Dolly::markers[Dolly::count - 1].time) {
						Dolly::update(0);
					}
					else if (*serverTime < Dolly::markers[0].time)
					{
						Cam->position = Dolly::markers[0].position;
						Cam->rotation = Math::LookAt(Dolly::markers[0].position, Dolly::markers[0].forward);
					}
					//else if (*serverTime < Dolly::markers[0].time && dollyStarted)
					//{
					//	Log::Info("Now before first marker. Stopping dolly");
					//	demo_stop_dolly(0);
					//}

					if (afterLastMarker())
					{
						demo_stop_dolly(0);
					}
				}
			}
		}

		lastTime = *serverTime;
	}
}

void Dolly::Initialise() {

	Log::Info("Dolly Cam Markers allocated at: %llx", &Dolly::markers);
	Log::Info("Alpha allocated at: %llx", &alpha);
	Log::Info("Start Time Real: %llx", &start_time_real);

	ConsoleCommands::Add("demo_add_marker", &demo_add_marker);
	ConsoleCommands::Add("demo_remove_marker", &demo_remove_marker);
	ConsoleCommands::Add("demo_remove_all", &demo_remove_all);
	ConsoleCommands::Add("demo_begin_dolly", &demo_begin_dolly);
	ConsoleCommands::Add("demo_stop_dolly", &demo_stop_dolly);
	ConsoleCommands::Add("demo_update_dolly", &Dolly::update);
	ConsoleCommands::Add("demo_test", &demo_test);

	CreateThread(0, 0, &Dolly::Loop, 0, 0, 0);
}


namespace DollyCam
{
	CamNode* head = NULL;
	CamNode* current_node = NULL;
	bool bplay = false;
	long long speed_tick = 60;
	long long current_tick_dolly = 0;
	long long begin_tick = 0;
	unsigned long* tickTime;

	static __int64* TEBAddress;
	static __int64 hModule;
	__int32 TlsIndex;

	void Init(uintptr_t module, uintptr_t teb)
	{
		hModule = module;
		TEBAddress = (__int64*)teb;
		TlsIndex = *(__int32*)(hModule + 0xA2A09C);
		tickTime = (unsigned long*)(*(__int64*)(*(__int64*)(TEBAddress + TlsIndex) + 0xC8i64) + 0xC);

		CamNode* node = head;
		CamNode* next = NULL;
		while (node != NULL)
		{
			next = node->next;
			free(node);
			node = next;
		}
		head = NULL;
		bplay = false;
		begin_tick = 0;
	}

	void MainFunction()
	{

		if (bplay)
		{
			if (Update(Halo::Cam,Halo::fov))
			{
				current_tick_dolly++;
			}
			else
			{
				bplay = false;
			}
		}
	}

	bool Update(Camera* Cam, float* fov)
	{
		CamNode *m0, *m1, *m2, *m3;
		CamNode *node = NULL;
		int i = 0;

		node = head;
		while (node != NULL)
		{
			if (node->t->time_relative <= current_tick_dolly)
				current_node = node;
			node = node->next;
		}

		if (current_node == NULL || current_node->next == NULL || Cam == nullptr || fov == nullptr) return false;

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
		float alpha;
		if ((current_node->next->t->time_relative - current_node->t->time_relative) == 0)
		{
			alpha = current_time_relative / 1.0;
		}
		else
		{
			alpha = current_time_relative / (float)(current_node->next->t->time_relative - current_node->t->time_relative);
		}
		 

		//lerp position
		Cam->position = Math::CatmullRomInterpolate(
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

		Cam->rotation = Math::LookAt(Cam->position, forward);

		//we just gonna use cosine interpolation for these because Catmullrom was being fucky. ill come back to this
		Cam->rotation.z = Math::CosineInterpolate(m1->t->roll, m2->t->roll, alpha);
		*fov = Math::CosineInterpolate(m1->t->fov, m2->t->fov, alpha);

		return true;
	}

	void AddMarker()
	{
		CameraMarker* _new_marker_ = new CameraMarker;
		CamNode* _new_node_ = new CamNode;
		long long current_tick = *tickTime;
		long long time_tick_relative = current_tick - begin_tick;

		Log::Debug("_new_node_:%llX\n", _new_node_);
		Log::Debug("Camera Address:%llX FOV Address:%llX\n", Cam, fov);

		memset(_new_marker_, 0, sizeof(CameraMarker));
		memset(_new_node_, 0, sizeof(CamNode));
		_new_node_->t = _new_marker_;
		//New Chain
		if (head == NULL)
		{
			begin_tick = current_tick;
			SetMarker(_new_marker_, current_tick);
			head = _new_node_;
			Log::Debug("Head:%llX\n", head);
			return;
		}
		//
		if (time_tick_relative < head->t->time_relative)
		{
			begin_tick = current_tick;
			SetMarker(_new_marker_, current_tick);
			CamNode* node = head;
			while (node != NULL)
			{
				node->t->time_relative += -time_tick_relative;
				node = node->next;
			}
			_new_node_->next = head;
			head = _new_node_;
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
		SetMarker(_new_marker_, current_tick);
	}

	bool RemoveNode(CamNode* node)
	{
		if (node == NULL) return false;

		if (head == node) head = head->next;
		if (node->prev != NULL) node->prev->next = node->next;
		if (node->next != NULL) node->next->prev = node->prev;
		delete node->t;
		delete node;

		return true;
	}

	bool RemoveClosestNode()
	{
		CamNode* node = GetClosestNode();
		if (node == NULL)
		{
			Log::Info("Failed To Find Closest Node");
		}
		return RemoveNode(node);
	}

	bool BetweenMarkers()
	{
		CamNode* node = head;
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
		CamNode *node, *result;

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
		if (dist < 0.5)
			return node;
		return NULL;
	}

	CamNode* GetHeaderNode()
	{
		return head;
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

	void SetMarker(CameraMarker* cameraMarker, long long time_tick)
	{
		if (Cam == NULL) return;
		Camera c = *Cam;

		cameraMarker->time_tick = time_tick;// unsafe
		cameraMarker->time_relative = time_tick - begin_tick;
		cameraMarker->position = c.position;
		cameraMarker->forward = Math::GetForwardPosition(0.5f, c.position, c.rotation);
		cameraMarker->fov = *fov;
		cameraMarker->roll = c.rotation.z;
	}
	bool Playing()
	{
		return bplay;
	}
	void Play()
	{
		bplay = true;
		Log::Debug("Camera Address:%llX FOV Address:%llX Function Address:%llX\n", Cam, fov, Math::CatmullRomInterpolate);
	}
	void Pause()
	{
		bplay = false;
	}
	void Restart()
	{
		current_node = head;
		bplay = false;
		current_tick_dolly = 0;
		Update(Halo::Cam, Halo::fov);
	}
}