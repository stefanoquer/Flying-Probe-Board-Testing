#include "App.h"
#include <string>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
	vector<string> args(&argv[0], &argv[argc]);
	App::Get().Init(args);
	App::Get().Run();
	App::Get().Destroy();
	return 0;
}
