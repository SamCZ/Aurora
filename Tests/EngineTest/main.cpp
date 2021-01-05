#include <iostream>

#include <Aurora/AuroraEngine.hpp>
#include <Aurora/Aurora.hpp>

using namespace Aurora;
using namespace Aurora::App;

class TestGame : public FWindowGameContext
{
public:
    explicit TestGame(FWindowPtr window) : FWindowGameContext(std::move(window)) {}

    void Init() override
    {

    }

    void Update(double delta, double currentTime) override
    {

    }

    void Render() override
    {
        auto* pRTV = GetSwapChain()->GetCurrentBackBufferRTV();
        auto* pDSV = GetSwapChain()->GetDepthBufferDSV();
        AuroraEngine::ImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Clear the back buffer
        const float ClearColor[] = {0.350f, 0.350f, 0.350f, 1.0f};
        // Let the engine perform required state transitions
        AuroraEngine::ImmediateContext->ClearRenderTarget(pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        AuroraEngine::ImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);


    }
};

int main()
{
    AuroraEngine::Init();
    AuroraEngine::AddWindow<TestGame>(1280, 720, "EmberSky");
    return AuroraEngine::Run();
}