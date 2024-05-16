#pragma once

#include "../Interfaces/IBoard.h"
#include <memory>
#include <string>

class App
{
public:
	void Init(const std::vector<std::string> &args);

	void Run();

	void Destroy();

	static App &Get();

private:
	std::string m_inPath;
	std::string m_outPath;

	// Board
	std::shared_ptr<IBoard> m_board;
};
