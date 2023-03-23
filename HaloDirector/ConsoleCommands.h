#pragma once
#include <string>

typedef void(ConsoleCommand)(const char* arg);

static class ConsoleCommands
{
public:
	static void Initialise();
	static void Add(std::string name, ConsoleCommand* function);
};

