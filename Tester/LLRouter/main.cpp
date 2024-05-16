#include "App.h"
#include <exception>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
	vector<string> args(&argv[0], &argv[argc]);
	try
	{
		App::Get().Init(args);
		App::Get().Run();
		App::Get().Destroy();
	}
	catch (std::exception &e)
	{
		throw std::runtime_error(e.what());
	}
	return 0;
}
