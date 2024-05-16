#pragma once

#include <string>
#include <vector>

class App
{
public:
	void Init(std::vector<std::string> &args);

	void Run();

	void Destroy();

	static App &Get();

private:
	std::string m_inPath;
	std::string m_outPath;
	bool m_skipJoin = false;
};
