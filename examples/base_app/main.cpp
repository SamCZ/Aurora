#include <Aurora/Aurora.hpp>
#include <Aurora/Resource/ResourceManager.hpp>

#include <Aurora/Graphics/Material/MaterialDefinition.hpp>

using namespace Aurora;

class BaseAppContext : public AppContext
{
	void Init() override
	{
		/*MaterialDefinition* matdef = GetEngine()->GetResourceManager()->LoadMaterialDef("Assets/Materials/Base/PBR.matd");

		Material* matInst = matdef->CreateInstance();

		matInst->SetTexture("Albedo", "Assets/Textures/stone.png");

		Material* matInst2 = matInst->Clone();

		Material* matInstDirect = GetEngine()->GetResourceManager()->LoadMaterial("Assets/Materials/BlueprintPBR.mat");

		matInstDirect->SetMacroSetState("Test", true);
		matInstDirect->SetRawMacro("yes", 1);*/

		MaterialDefinitionDesc materialDefinitionDesc;
		materialDefinitionDesc.Name = "TestDesc";
		materialDefinitionDesc.Filepath = "[[RUNTIME/desc]]";
		materialDefinitionDesc.ShaderPasses[(PassType_t)EPassType::Ambient] = GetEngine()->GetResourceManager()->CreateShaderProgramDesc("PBR", {
			{EShaderType::Vertex, "Assets/Shaders/World/PBRBasic/ambient.vss"},
			{EShaderType::Pixel, "Assets/Shaders/World/PBRBasic/ambient.fss"}
		});

		MaterialDefinition matDef(materialDefinitionDesc);

		std::shared_ptr<SMaterial> mat = matDef.CreateInstance();
	}

	void Update(double delta) override
	{

	}

	void Render() override
	{

	}

	void RenderVg() override
	{

	}
};

int main()
{
	WindowDefinition windowDefinition = {};
	windowDefinition.Width = 1270;
	windowDefinition.Height = 720;
	windowDefinition.HasOSWindowBorder = true;
	windowDefinition.Maximized = false;
	windowDefinition.Title = "BaseApp";

	Aurora::AuroraEngine engine;
	engine.Init(new BaseAppContext(), windowDefinition);
	engine.Run();
	return 0;
}