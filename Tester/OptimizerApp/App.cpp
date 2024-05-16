#include "App.h"
#include "../HLRouter/Board.h"
#include "../Joiner/Board.h"
#include <filesystem>
#include <iostream>

using namespace std;

void App::Init(std::vector<std::string> &args)
{
	if (args.size() < 3)
	{
		cout << "Usage: OptimizerApp <input_path> <output_path>" << endl;
		// throw runtime_error("Too few arguments!");
		args.emplace_back("/Users/andrea/CLionProjects/Probes/Application/board");
		args.emplace_back("/Users/andrea/CLionProjects/Probes/Application/optimized");
	}
	if (args.size() > 3)
	{
		m_skipJoin = true;
	}
	m_inPath = args[1];
	m_outPath = args[2];
	filesystem::create_directory(m_outPath);
}

void App::Run()
{
	if (!m_skipJoin)
	{
		{
			cout << "------BEGIN JOIN TASK------" << endl;
			Joiner::Board board;
			board.ReadFromFiles(m_inPath);
			board.Optimize();
			board.SaveTo(m_outPath);
			cout << "------END JOIN TASK------" << endl;
		}
		if (filesystem::exists(m_outPath + "/board.txt"))
		{
			filesystem::remove(m_outPath + "/board.txt");
		}
		filesystem::copy(m_inPath + "/board.txt", m_outPath + "/board.txt");
	}
	{
		cout << "------BEGIN ROUTING TASK------" << endl;
		HLRouter::Board board;
		board.ReadFromFiles(m_outPath);
		board.Optimize();
		board.SaveTo(m_outPath);
		cout << "------END ROUTING TASK------" << endl;
	}
}

void App::Destroy()
{
}

App &App::Get()
{
	static App app;
	return app;
}
