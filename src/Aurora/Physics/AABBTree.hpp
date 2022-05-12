#pragma once

#include <memory>
#include <vector>
#include <map>
#include <forward_list>
#include <stack>

#include "Aurora/Core/Types.hpp"
#include "Aurora/Physics/AABB.hpp"

#define AABB_NULL_NODE 0xffffffff

namespace Aurora
{
	typedef uint32_t NodeIndex_t;

	template<typename T>
	struct AABBNode
	{
		AABB Bounds;
		T* Object;
		// tree links
		NodeIndex_t ParentNodeIndex;
		NodeIndex_t LeftNodeIndex;
		NodeIndex_t RightNodeIndex;
		// node linked list link
		NodeIndex_t NextNodeIndex;

		bool IsLeaf() const { return LeftNodeIndex == AABB_NULL_NODE; }

		AABBNode() : Object(nullptr), ParentNodeIndex(AABB_NULL_NODE), LeftNodeIndex(AABB_NULL_NODE), RightNodeIndex(AABB_NULL_NODE), NextNodeIndex(AABB_NULL_NODE) {}
	};

	template<typename T>
	class AABBTree
	{
	private:
		std::map<T*, NodeIndex_t> m_ObjectNodeIndexMap;
		std::vector<AABBNode<T>> m_Nodes;

		NodeIndex_t m_RootNodeIndex;
		NodeIndex_t m_NextFreeNodeIndex;

		uint32_t m_AllocatedNodeCount;
		uint32_t m_NodeCapacity;
		uint32_t m_GrowthSize;
	public:
		explicit AABBTree(uint32_t initialSize) : m_RootNodeIndex(AABB_NULL_NODE), m_AllocatedNodeCount(0), m_NextFreeNodeIndex(0), m_NodeCapacity(initialSize), m_GrowthSize(initialSize)
		{
			m_Nodes.resize(initialSize);
			for (unsigned nodeIndex = 0; nodeIndex < initialSize; nodeIndex++)
			{
				AABBNode<T>& node = m_Nodes[nodeIndex];
				node.NextNodeIndex = nodeIndex + 1;
			}
			m_Nodes[initialSize-1].NextNodeIndex = AABB_NULL_NODE;
		}

		const std::vector<AABBNode<T>>& GetNodes() const { return m_Nodes; }

		void InsertObject(T* object, const AABB& aabb)
		{
			NodeIndex_t nodeIndex = AllocateNode();
			AABBNode<T>& node = m_Nodes[nodeIndex];

			node.Bounds = aabb;
			node.Object = object;

			InsertLeaf(nodeIndex);
			m_ObjectNodeIndexMap[object] = nodeIndex;
		}

		void RemoveObject(T* object)
		{
			NodeIndex_t nodeIndex = m_ObjectNodeIndexMap[object];
			RemoveLeaf(nodeIndex);
			DeallocateNode(nodeIndex);
			m_ObjectNodeIndexMap.erase(object);
		}

		void UpdateObject(T* object, const AABB& aabb)
		{
			NodeIndex_t nodeIndex = m_ObjectNodeIndexMap[object];
			UpdateLeaf(nodeIndex, aabb);
		}

		std::forward_list<T*> QueryOverlaps(T* object, const AABB& aabb) const
		{
			std::forward_list<T*> overlaps;
			std::stack<NodeIndex_t> stack;
			AABB testAabb = aabb;

			stack.push(m_RootNodeIndex);
			while(!stack.empty())
			{
				NodeIndex_t nodeIndex = stack.top();
				stack.pop();

				if (nodeIndex == AABB_NULL_NODE) continue;

				const AABBNode<T>& node = m_Nodes[nodeIndex];
				if (node.Bounds.Overlaps(testAabb))
				{
					if (node.IsLeaf() && node.Object != object)
					{
						overlaps.push_front(node.Object);
					}
					else
					{
						stack.push(node.LeftNodeIndex);
						stack.push(node.RightNodeIndex);
					}
				}
			}

			return overlaps;
		}
	private:
		NodeIndex_t AllocateNode()
		{
			// if we have no free tree nodes then grow the pool
			if (m_NextFreeNodeIndex == AABB_NULL_NODE)
			{
				assert(m_AllocatedNodeCount == m_NodeCapacity);

				m_NodeCapacity += m_GrowthSize;
				m_Nodes.resize(m_NodeCapacity);
				for (NodeIndex_t nodeIndex = m_AllocatedNodeCount; nodeIndex < m_NodeCapacity; nodeIndex++)
				{
					AABBNode<T>& node = m_Nodes[nodeIndex];
					node.NextNodeIndex = nodeIndex + 1;
				}
				m_Nodes[m_NodeCapacity - 1].NextNodeIndex = AABB_NULL_NODE;
				m_NextFreeNodeIndex = m_AllocatedNodeCount;
			}

			NodeIndex_t nodeIndex = m_NextFreeNodeIndex;
			AABBNode<T>& allocatedNode = m_Nodes[nodeIndex];
			allocatedNode.ParentNodeIndex = AABB_NULL_NODE;
			allocatedNode.LeftNodeIndex = AABB_NULL_NODE;
			allocatedNode.RightNodeIndex = AABB_NULL_NODE;
			m_NextFreeNodeIndex = allocatedNode.NextNodeIndex;
			m_AllocatedNodeCount++;

			return nodeIndex;
		}

		void DeallocateNode(NodeIndex_t nodeIndex)
		{
			AABBNode<T>& deallocatedNode = m_Nodes[nodeIndex];
			deallocatedNode.NextNodeIndex = m_NextFreeNodeIndex;
			m_NextFreeNodeIndex = nodeIndex;
			m_AllocatedNodeCount--;
		}

		void InsertLeaf(NodeIndex_t leafNodeIndex)
		{
			// make sure we're inserting a new leaf
			assert(m_Nodes[leafNodeIndex].ParentNodeIndex == AABB_NULL_NODE);
			assert(m_Nodes[leafNodeIndex].LeftNodeIndex == AABB_NULL_NODE);
			assert(m_Nodes[leafNodeIndex].RightNodeIndex == AABB_NULL_NODE);

			// if the tree is empty then we make the root the leaf
			if (m_RootNodeIndex == AABB_NULL_NODE)
			{
				m_RootNodeIndex = leafNodeIndex;
				return;
			}

			// search for the best place to put the new leaf in the tree
			// we use surface area and depth as search heuristics
			NodeIndex_t treeNodeIndex = m_RootNodeIndex;
			AABBNode<T>& leafNode = m_Nodes[leafNodeIndex];
			while (!m_Nodes[treeNodeIndex].IsLeaf())
			{
				// because of the test in the while loop above we know we are never a leaf inside it
				const AABBNode<T>& treeNode = m_Nodes[treeNodeIndex];
				NodeIndex_t leftNodeIndex = treeNode.LeftNodeIndex;
				NodeIndex_t rightNodeIndex = treeNode.RightNodeIndex;
				const AABBNode<T>& leftNode = m_Nodes[leftNodeIndex];
				const AABBNode<T>& rightNode = m_Nodes[rightNodeIndex];

				AABB combinedAabb = treeNode.Bounds.Merge(leafNode.Bounds);

				float newParentNodeCost = 2.0f * combinedAabb.GetSurfaceArea();
				float minimumPushDownCost = 2.0f * (combinedAabb.GetSurfaceArea() - treeNode.Bounds.GetSurfaceArea());

				// use the costs to figure out whether to create a new parent here or descend
				float costLeft;
				float costRight;
				if (leftNode.IsLeaf())
				{
					costLeft = leafNode.Bounds.Merge(leftNode.Bounds).GetSurfaceArea() + minimumPushDownCost;
				}
				else
				{
					AABB newLeftAabb = leafNode.Bounds.Merge(leftNode.Bounds);
					costLeft = (newLeftAabb.GetSurfaceArea() - leftNode.Bounds.GetSurfaceArea()) + minimumPushDownCost;
				}
				if (rightNode.IsLeaf())
				{
					costRight = leafNode.Bounds.Merge(rightNode.Bounds).GetSurfaceArea() + minimumPushDownCost;
				}
				else
				{
					AABB newRightAabb = leafNode.Bounds.Merge(rightNode.Bounds);
					costRight = (newRightAabb.GetSurfaceArea() - rightNode.Bounds.GetSurfaceArea()) + minimumPushDownCost;
				}

				// if the cost of creating a new parent node here is less than descending in either direction then
				// we know we need to create a new parent node, errrr, here and attach the leaf to that
				if (newParentNodeCost < costLeft && newParentNodeCost < costRight)
				{
					break;
				}

				// otherwise descend in the cheapest direction
				if (costLeft < costRight)
				{
					treeNodeIndex = leftNodeIndex;
				}
				else
				{
					treeNodeIndex = rightNodeIndex;
				}
			}

			// the leafs sibling is going to be the node we found above and we are going to create a new
			// parent node and attach the leaf and this item
			NodeIndex_t leafSiblingIndex = treeNodeIndex;
			AABBNode<T>& leafSibling = m_Nodes[leafSiblingIndex];
			NodeIndex_t oldParentIndex = leafSibling.ParentNodeIndex;
			NodeIndex_t newParentIndex = AllocateNode();
			AABBNode<T>& newParent = m_Nodes[newParentIndex];
			newParent.ParentNodeIndex = oldParentIndex;
			newParent.Bounds = leafNode.Bounds.Merge(leafSibling.Bounds); // the new parents aabb is the leaf aabb combined with it's siblings aabb
			newParent.LeftNodeIndex = leafSiblingIndex;
			newParent.RightNodeIndex = leafNodeIndex;
			leafNode.ParentNodeIndex = newParentIndex;
			leafSibling.ParentNodeIndex = newParentIndex;

			if (oldParentIndex == AABB_NULL_NODE)
			{
				// the old parent was the root and so this is now the root
				m_RootNodeIndex = newParentIndex;
			}
			else
			{
				// the old parent was not the root and so we need to patch the left or right index to
				// point to the new node
				AABBNode<T>& oldParent = m_Nodes[oldParentIndex];
				if (oldParent.LeftNodeIndex == leafSiblingIndex)
				{
					oldParent.LeftNodeIndex = newParentIndex;
				}
				else
				{
					oldParent.RightNodeIndex = newParentIndex;
				}
			}

			// finally we need to walk back up the tree fixing heights and areas
			treeNodeIndex = leafNode.ParentNodeIndex;
			FixUpwardsTree(treeNodeIndex);
		}

		void RemoveLeaf(NodeIndex_t leafNodeIndex)
		{
			// if the leaf is the root then we can just clear the root pointer and return
			if (leafNodeIndex == m_RootNodeIndex)
			{
				m_RootNodeIndex = AABB_NULL_NODE;
				return;
			}

			AABBNode<T>& leafNode = m_Nodes[leafNodeIndex];
			NodeIndex_t parentNodeIndex = leafNode.ParentNodeIndex;
			const AABBNode<T>& parentNode = m_Nodes[parentNodeIndex];
			NodeIndex_t grandParentNodeIndex = parentNode.ParentNodeIndex;
			NodeIndex_t siblingNodeIndex = parentNode.LeftNodeIndex == leafNodeIndex ? parentNode.RightNodeIndex : parentNode.LeftNodeIndex;
			assert(siblingNodeIndex != AABB_NULL_NODE); // we must have a sibling
			AABBNode<T>& siblingNode = m_Nodes[siblingNodeIndex];

			if (grandParentNodeIndex != AABB_NULL_NODE)
			{
				// if we have a grand parent (i.e. the parent is not the root) then destroy the parent and connect the sibling to the grandparent in its
				// place
				AABBNode<T>& grandParentNode = m_Nodes[grandParentNodeIndex];
				if (grandParentNode.LeftNodeIndex == parentNodeIndex)
				{
					grandParentNode.LeftNodeIndex = siblingNodeIndex;
				}
				else
				{
					grandParentNode.RightNodeIndex = siblingNodeIndex;
				}
				siblingNode.ParentNodeIndex = grandParentNodeIndex;
				DeallocateNode(parentNodeIndex);

				FixUpwardsTree(grandParentNodeIndex);
			}
			else
			{
				// if we have no grandparent then the parent is the root and so our sibling becomes the root and has it's parent removed
				m_RootNodeIndex = siblingNodeIndex;
				siblingNode.ParentNodeIndex = AABB_NULL_NODE;
				DeallocateNode(parentNodeIndex);
			}

			leafNode.ParentNodeIndex = AABB_NULL_NODE;
		}

		void UpdateLeaf(NodeIndex_t leafNodeIndex, const AABB& newAaab)
		{
			AABBNode<T>& node = m_Nodes[leafNodeIndex];

			// if the node contains the new aabb then we just leave things
			// TODO: when we add velocity this check should kick in as often an update will lie within the velocity fattened initial aabb
			// to support this we might need to differentiate between velocity fattened aabb and actual aabb
			if (node.Bounds.Contains(newAaab)) return;

			RemoveLeaf(leafNodeIndex);
			node.Bounds = newAaab;
			InsertLeaf(leafNodeIndex);
		}

		void FixUpwardsTree(NodeIndex_t treeNodeIndex)
		{
			while (treeNodeIndex != AABB_NULL_NODE)
			{
				AABBNode<T>& treeNode = m_Nodes[treeNodeIndex];

				// every node should be a parent
				assert(treeNode.LeftNodeIndex != AABB_NULL_NODE && treeNode.RightNodeIndex != AABB_NULL_NODE);

				// fix height and area
				const AABBNode<T>& leftNode = m_Nodes[treeNode.LeftNodeIndex];
				const AABBNode<T>& rightNode = m_Nodes[treeNode.RightNodeIndex];
				treeNode.Bounds = leftNode.Bounds.Merge(rightNode.Bounds);

				treeNodeIndex = treeNode.ParentNodeIndex;
			}
		}
	};
}
