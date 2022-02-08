#include <Aurora/Aurora.hpp>
#include <Aurora/Resource/ResourceManager.hpp>

#include <Aurora/Graphics/Material/MaterialDefinition.hpp>
#include <Aurora/Graphics/Material/SMaterial.hpp>

#include <Shaders/World/PBRBasic/cb_pbr.h>

using namespace Aurora;

class BaseAppContext : public AppContext
{
	void Init() override
	{
		MaterialDefinitionDesc materialDefinitionDesc;
		materialDefinitionDesc.Name = "TestDesc";
		materialDefinitionDesc.Filepath = "[[RUNTIME/desc]]";
		materialDefinitionDesc.ShaderPasses[(PassType_t)EPassType::Ambient] = GetEngine()->GetResourceManager()->CreateShaderProgramDesc("PBR", {
			{EShaderType::Vertex, "Assets/Shaders/World/PBRBasic/ambient.vss"},
			{EShaderType::Pixel, "Assets/Shaders/World/PBRBasic/ambient.fss"}
		});
		materialDefinitionDesc.ShaderPasses[(PassType_t)EPassType::Depth] = GetEngine()->GetResourceManager()->CreateShaderProgramDesc("PBRDepth", {
			{EShaderType::Vertex, "Assets/Shaders/World/PBRBasic/depth.vss"},
			{EShaderType::Pixel, "Assets/Shaders/World/PBRBasic/depth.fss"}
		});

		MaterialDefinition matDef(materialDefinitionDesc);

		std::shared_ptr<SMaterial> mat = matDef.CreateInstance();

		if(PBRConstants* matConstants = mat->GetVarBlock<PBRConstants>("PBRConstants"_HASH))
		{
			matConstants->AmbientOcclusion = 0.5f;
			matConstants->Metallic = 1.0f;
			matConstants->EmissionFactor = Vector4(1, 2, 3, 4);
		}

		mat->SetVariable("AmbientOcclusion"_HASH, 0.6f);

		float metallic;
		if(mat->GetVariable<float>("Metallic"_HASH, metallic))
		{
			std::cout << "metallic: " << metallic << std::endl;
		}

		Vector4 emf;
		if(mat->GetVariable<Vector4>("EmissionFactor"_HASH, emf))
		{
			std::cout << glm::to_string(emf) << std::endl;
		}
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