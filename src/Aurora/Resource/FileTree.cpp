#include "FileTree.hpp"

namespace Aurora
{

	PathNode *PathNode::Find(const std::filesystem::path &path)
	{
		if (Path == path)
		{
			return this;
		}

		PathNode* foundNode = nullptr;

		for (PathNode& node : Childrens)
		{
			foundNode = node.Find(path);

			if (foundNode)
				break;
		}

		return foundNode;
	}

	PathNode *PathNode::FindHomeForPath(const std::filesystem::path &path)
	{
		std::filesystem::path currentPath = path;
		PathNode* currentNode;

		if (!std::filesystem::is_directory(currentPath))
		{
			currentPath = currentPath.parent_path();
		}

		do {
			currentNode = Find(currentPath);
			currentPath = currentPath.parent_path();
		} while(currentNode == nullptr && !currentPath.empty());

		return currentNode;
	}

	bool PathNode::AddFile(const std::filesystem::path &path)
	{
		PathNode* homeFolder = FindHomeForPath(path);

		if (!homeFolder)
			return false;

		homeFolder->Childrens.emplace_back(PathNode(path, std::filesystem::is_directory(path), homeFolder, {}));
		return true;
	}

	bool PathNode::RemoveFile(const std::filesystem::path &path)
	{
		PathNode* homeFolder = FindHomeForPath(path);

		if (!homeFolder)
			return false;

		if(*homeFolder == path)
		{
			homeFolder = homeFolder->ParentNode;
		}

		if (!homeFolder)
			return false;

		homeFolder->Childrens.erase(std::find(homeFolder->Childrens.begin(), homeFolder->end(), path));
		return true;
	}

	bool PathNode::RenameFile(const std::filesystem::path &newPath, const std::filesystem::path &oldNamePath)
	{
		PathNode* node = Find(oldNamePath);

		if (!node)
		{
			AU_LOG_INFO("Cound not rename file because it does not exists in filetree ", oldNamePath);
			return false;
		}

		node->Childrens.clear();
		node->Path = newPath;
		node->IsDirectory = std::filesystem::is_directory(newPath);
		node->Traverse();
		return true;
	}

	void PathNode::Traverse()
	{
		if(!IsDirectory)
			return;

		for (auto& dirIt : std::filesystem::directory_iterator(Path))
		{
			const std::filesystem::path& path = dirIt.path();

			PathNode node(
				path,
				dirIt.is_directory(),
				this,
				{}
			);

			Childrens.emplace_back(node).Traverse();
		}
	}
}