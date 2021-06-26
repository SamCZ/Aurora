#include <Aurora/AuroraEngine.hpp>

using namespace Aurora;

class BaseAppContext : public WindowGameContext
{
public:
	explicit BaseAppContext(const IWindow_ptr &window) : WindowGameContext(window)
	{

	}

	void Init() override
	{

	}

	void Update(double delta, double currentTime) override
	{

	}

	void Render() override
	{

	}
};

int main()
{
	AuroraEngine::Init();

	AuroraEngine::AddWindow<BaseAppContext>(1280, 720, "Aurora - Base App", true);

	return AuroraEngine::Run();
}