#pragma once

#include <utility>

#include "Aurora/Core/Types.hpp"

namespace Aurora
{
	struct PathNode
	{
		std::filesystem::path Path;
		bool IsDirectory;
		PathNode* ParentNode;
		std::vector<PathNode> Childrens;

		PathNode() : Path(), IsDirectory(true), Childrens(), ParentNode(nullptr)
		{}

		PathNode(std::filesystem::path path, bool isDirectory, PathNode *parentNode, std::vector<PathNode> childrens) : Path(std::move(path)), IsDirectory(isDirectory), ParentNode(parentNode),
			Childrens(std::move(childrens))
		{}

		bool operator==(const PathNode& other) const { return Path == other.Path; }
		bool operator==(const std::filesystem::path& other) const { return Path == other; }

		std::vector<PathNode>::iterator begin() { return Childrens.begin(); }
		std::vector<PathNode>::iterator end() { return Childrens.end(); }

		[[nodiscard]] std::vector<PathNode>::const_iterator begin() const { return Childrens.begin(); }
		[[nodiscard]] std::vector<PathNode>::const_iterator end() const { return Childrens.end(); }

		PathNode* Find(const std::filesystem::path& path);
		PathNode* FindHomeForPath(const std::filesystem::path& path);

		bool AddFile(const std::filesystem::path& path);
		bool RemoveFile(const std::filesystem::path& path);
		bool RenameFile(const std::filesystem::path& newPath, const std::filesystem::path& oldNamePath);

		void SearchFor(const std::string& searchString, std::vector<PathNode>& foundFiles, bool includeDirectories) const;

		void Traverse();
	};

	struct FileTree : PathNode
	{
		explicit FileTree(const std::filesystem::path& root) : PathNode(root, std::filesystem::is_directory(root), nullptr, {})
		{
			Traverse();
		}
	};
}