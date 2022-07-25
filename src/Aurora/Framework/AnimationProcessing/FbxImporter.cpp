#include "FbxImporter.hpp"

#include <Aurora/Logger/Logger.hpp>
#include "FbxUtil.hpp"
#include "FbxMemoryStream.hpp"

namespace FbxImport
{
	void LoadScene(const std::vector<uint8_t>& data)
	{
		AU_LOG_INFO(FbxManager::GetVersion(true));

		FbxManager* fbxManager = FbxManager::Create();
		auto* settings = FbxIOSettings::Create(fbxManager, IOSROOT);

		//Load plugins from the executable directory (optional)
		FbxString lPath = FbxGetApplicationDirectory();
		fbxManager->LoadPluginsDirectory(lPath.Buffer());

		FbxImporter* importer = FbxImporter::Create(fbxManager, "");

		MemoryFbxStream stream(data);

		int a = 5;

		const bool hasInitialized = importer->Initialize(&stream, &a, -1, settings);

		int fileVersion[3];
		importer->GetFileVersion(fileVersion[0], fileVersion[1], fileVersion[2]);

		if (not hasInitialized)
		{
			FbxString error = importer->GetStatus().GetErrorString();
			AU_LOG_WARNING("Call to FbxImporter::Initialize() failed.");
			AU_LOG_WARNING("Error returned:", error.Buffer());

			if (importer->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
			{
				int SdkVersion[3];
				FbxManager::GetFileFormatVersion(SdkVersion[0], SdkVersion[1], SdkVersion[2]);

				AU_LOG_WARNING("FBX file format version for this FBX SDK is ", SdkVersion[0], ".", SdkVersion[1], ".", SdkVersion[2]);
				AU_LOG_WARNING("FBX file format version for the file is  ", fileVersion[0], ".", fileVersion[1], ".", fileVersion[2]);
			}

			return;
		}

		FbxScene* scene = FbxScene::Create(fbxManager, "");
		if (not importer->Import(scene))
		{
			AU_LOG_WARNING(importer->GetStatus().GetErrorString());
			return;
		}
		FbxAxisSystem::OpenGL.DeepConvertScene(scene);

		FbxGeometryConverter{fbxManager}.Triangulate(scene, true);

		FbxNode* const root_node = scene->GetRootNode();

		if (root_node == nullptr)
		{
			AU_LOG_WARNING("An FBX scene did not contain a root node.");
			return;
		}

		for (int i = 0; i < root_node->GetChildCount(); ++i)
		{
			AU_LOG_INFO(root_node->GetChild(i)->GetName());
		}

		scene->Destroy();
		importer->Destroy();
		fbxManager->Destroy();
	}
}