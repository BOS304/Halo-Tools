#include "ConsoleCommands.h"
#include "StdInc.h"
#include <unordered_map>
#include <string>
#include <sstream>
std::unordered_map<std::string, ConsoleCommand*> umap;

bool has_key(std::string key)
{
	// Key is not present 
	if (umap.find(key) == umap.end())
		return false;

	return true;
}

std::string convertToString(char* a, int size)
{
	int i;
	std::string s = "";
	for (i = 0; i < size; i++) {
		if (a[i] != (char)0)
			s = s + a[i];
	}
	return s;
}

std::vector<std::string> split(const std::string& str, const std::string& delim)
{
	std::vector<std::string> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == std::string::npos) pos = str.length();
		std::string token = str.substr(prev, pos - prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}

void AddText(const char* buffer, int size)
{
	std::string s = convertToString((char*)buffer, size);
	std::vector<std::string> segments = split(s, " ");

	if (segments.size() > 0)
	{

		if (segments.size() == 1)
		{
			if (has_key(segments[0]))
			{
				umap[segments[0]](NULL);
			}
			else {
				Log::Error("Function not found: %s", segments[0]);
			}
		}
		else if (segments.size() == 2)
		{
			if (has_key(segments[0]))
			{
				umap[segments[0]](segments[1].c_str());
			}
			else {
				Log::Error("Function not found: %s", segments[0].c_str());
			}
		}
		else {
			Log::Error("Invalid Console Command: %s", buffer);
		}
	}
}

DWORD WINAPI CommandLoop(LPVOID Param) {
	while (true)
	{
		char buffer[255];
		size_t sizeOut;
		_cgets_s<255>(buffer, &sizeOut);
		
		AddText(buffer, (int)sizeOut);
	}
}

void ConsoleCommands::Add(std::string name, ConsoleCommand* function)
{
	umap[name] = function;
}

void help(const char* arg)
{
	for (auto it = umap.begin(); it != umap.end(); it++)
	{
		Log::Info("%s", it->first);
	}
}

void cls(const char* arg)
{
	system("cls");
}

void ConsoleCommands::Initialise()
{
	ConsoleCommands::Add("help", help);
	ConsoleCommands::Add("cls", cls);
	DollyCam::Console::Init();
	Halo::Console::Init();
	CreateThread(0, 0, &CommandLoop, 0, 0, 0);
}