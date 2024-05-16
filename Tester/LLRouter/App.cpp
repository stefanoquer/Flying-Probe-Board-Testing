#include "App.h"

#include "Board/Board.h"
#include <filesystem>
#include <iostream>

using namespace std;

void App::Init(const std::vector<std::string> &args)
{
	if (args.size() < 3)
	{
		cout << "Usage: HLRouter <input_path> <output_path>" << endl;
		throw runtime_error("Too few arguments!");
	}
	m_inPath = args[1];
	m_outPath = args[2];
	if (!filesystem::exists(m_outPath))
	{
		filesystem::create_directory(m_outPath);
	}
	m_board = std::make_shared<LLRouter::Board>();
}

void App::Run()
{
	m_board->ReadFromFiles(m_inPath);
	m_board->Optimize();
	m_board->SaveTo(m_outPath);
}

void App::Destroy()
{
	m_board = nullptr;
}
App &App::Get()
{
	static App app;
	return app;
}
