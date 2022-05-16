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
		AABB aabb;
		T* Object;
		// tree links
		unsigned parentNodeIndex;
		unsigned leftNodeIndex;
		unsigned rightNodeIndex;
		// node linked list link
		unsigned nextNodeIndex;

		[[nodiscard]] bool IsLeaf() const { return leftNodeIndex == AABB_NULL_NODE; }

		AABBNode() : Object(nullptr), parentNodeIndex(AABB_NULL_NODE), leftNodeIndex(AABB_NULL_NODE), rightNodeIndex(AABB_NULL_NODE), nextNodeIndex(AABB_NULL_NODE)
		{

		}
	};

	template<typename T>
	class AABBTree
	{
	private:
		std::map<T*, unsigned> _objectNodeIndexMap;
		std::vector<AABBNode<T>> _nodes;
		unsigned _rootNodeIndex;
		unsigned _allocatedNodeCount;
		unsigned _nextFreeNodeIndex;
		unsigned _nodeCapacity;
		unsigned _growthSize;
	public:
		AABBTree(unsigned initialSize) : _rootNodeIndex(AABB_NULL_NODE), _allocatedNodeCount(0), _nextFreeNodeIndex(0), _nodeCapacity(initialSize), _growthSize(initialSize)
		{
			_nodes.resize(initialSize);
			for (unsigned nodeIndex = 0; nodeIndex < initialSize; nodeIndex++)
			{
				AABBNode<T>& node = _nodes[nodeIndex];
				node.nextNodeIndex = nodeIndex + 1;
			}
			_nodes[initialSize-1].nextNodeIndex = AABB_NULL_NODE;
		}

		const std::vector<AABBNode<T>>& GetNodes() const { return _nodes; }

		void InsertObject(T* Object, const AABB& aabb)
		{
			unsigned nodeIndex = allocateNode();
			AABBNode<T>& node = _nodes[nodeIndex];

			node.aabb = aabb;
			node.Object = Object;

			insertLeaf(nodeIndex);
			_objectNodeIndexMap[Object] = nodeIndex;
		}

		void RemoveObject(T* Object)
		{
			unsigned nodeIndex = _objectNodeIndexMap[Object];
			RemoveLeaf(nodeIndex);
			deallocateNode(nodeIndex);
			_objectNodeIndexMap.erase(Object);
		}

		void UpdateObject(T* Object, const AABB& aabb)
		{
			unsigned nodeIndex = _objectNodeIndexMap[Object];
			UpdateLeaf(nodeIndex, aabb);
		}

		std::forward_list<T*> QueryOverlaps(T* Object, const AABB& aabb) const
		{
			std::forward_list<T*> overlaps;
			std::stack<unsigned> stack;
			AABB testAabb = aabb;

			stack.push(_rootNodeIndex);
			while(!stack.empty())
			{
				unsigned nodeIndex = stack.top();
				stack.pop();

				if (nodeIndex == AABB_NULL_NODE) continue;

				const AABBNode<T>& node = _nodes[nodeIndex];
				if (node.aabb.Overlaps(testAabb))
				{
					if (node.IsLeaf() && node.Object != Object)
					{
						overlaps.push_front(node.Object);
					}
					else
					{
						stack.push(node.leftNodeIndex);
						stack.push(node.rightNodeIndex);
					}
				}
			}

			return overlaps;
		}
	private:
		unsigned allocateNode()
		{
			// if we have no free tree nodes then grow the pool
			if (_nextFreeNodeIndex == AABB_NULL_NODE)
			{
				// FIXME: This does not work correctly

				assert(_allocatedNodeCount == _nodeCapacity);

				_nodeCapacity += _growthSize;
				_nodes.resize(_nodeCapacity);
				for (unsigned nodeIndex = _allocatedNodeCount; nodeIndex < _nodeCapacity; nodeIndex++)
				{
					AABBNode<T>& node = _nodes[nodeIndex];
					node.nextNodeIndex = nodeIndex + 1;
				}
				_nodes[_nodeCapacity - 1].nextNodeIndex = AABB_NULL_NODE;
				_nextFreeNodeIndex = _allocatedNodeCount;
			}

			unsigned nodeIndex = _nextFreeNodeIndex;
			AABBNode<T>& allocatedNode = _nodes[nodeIndex];
			allocatedNode.parentNodeIndex = AABB_NULL_NODE;
			allocatedNode.leftNodeIndex = AABB_NULL_NODE;
			allocatedNode.rightNodeIndex = AABB_NULL_NODE;
			_nextFreeNodeIndex = allocatedNode.nextNodeIndex;
			_allocatedNodeCount++;

			return nodeIndex;
		}

		void deallocateNode(unsigned nodeIndex)
		{
			AABBNode<T>& deallocatedNode = _nodes[nodeIndex];
			deallocatedNode.nextNodeIndex = _nextFreeNodeIndex;
			_nextFreeNodeIndex = nodeIndex;
			_allocatedNodeCount--;
		}

		void insertLeaf(unsigned leafNodeIndex)
		{
			// make sure we're inserting a new leaf
			assert(_nodes[leafNodeIndex].parentNodeIndex == AABB_NULL_NODE);
			assert(_nodes[leafNodeIndex].leftNodeIndex == AABB_NULL_NODE);
			assert(_nodes[leafNodeIndex].rightNodeIndex == AABB_NULL_NODE);

			// if the tree is empty then we make the root the leaf
			if (_rootNodeIndex == AABB_NULL_NODE)
			{
				_rootNodeIndex = leafNodeIndex;
				return;
			}

			// search for the best place to put the new leaf in the tree
			// we use surface area and depth as search heuristics
			unsigned treeNodeIndex = _rootNodeIndex;
			AABBNode<T>& leafNode = _nodes[leafNodeIndex];
			while (!_nodes[treeNodeIndex].IsLeaf())
			{
				// because of the test in the while loop above we know we are never a leaf inside it
				const AABBNode<T>& treeNode = _nodes[treeNodeIndex];
				unsigned leftNodeIndex = treeNode.leftNodeIndex;
				unsigned rightNodeIndex = treeNode.rightNodeIndex;
				const AABBNode<T>& leftNode = _nodes[leftNodeIndex];
				const AABBNode<T>& rightNode = _nodes[rightNodeIndex];

				AABB combinedAabb = treeNode.aabb.Merge(leafNode.aabb);

				float newParentNodeCost = 2.0f * combinedAabb.GetSurfaceArea();
				float minimumPushDownCost = 2.0f * (combinedAabb.GetSurfaceArea() - treeNode.aabb.GetSurfaceArea());

				// use the costs to figure out whether to create a new parent here or descend
				float costLeft;
				float costRight;
				if (leftNode.IsLeaf())
				{
					costLeft = leafNode.aabb.Merge(leftNode.aabb).GetSurfaceArea() + minimumPushDownCost;
				}
				else
				{
					AABB newLeftAabb = leafNode.aabb.Merge(leftNode.aabb);
					costLeft = (newLeftAabb.GetSurfaceArea() - leftNode.aabb.GetSurfaceArea()) + minimumPushDownCost;
				}
				if (rightNode.IsLeaf())
				{
					costRight = leafNode.aabb.Merge(rightNode.aabb).GetSurfaceArea() + minimumPushDownCost;
				}
				else
				{
					AABB newRightAabb = leafNode.aabb.Merge(rightNode.aabb);
					costRight = (newRightAabb.GetSurfaceArea() - rightNode.aabb.GetSurfaceArea()) + minimumPushDownCost;
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
			unsigned leafSiblingIndex = treeNodeIndex;
			AABBNode<T>& leafSibling = _nodes[leafSiblingIndex];
			unsigned oldParentIndex = leafSibling.parentNodeIndex;
			unsigned newParentIndex = allocateNode();
			AABBNode<T>& newParent = _nodes[newParentIndex];
			newParent.parentNodeIndex = oldParentIndex;
			newParent.aabb = leafNode.aabb.Merge(leafSibling.aabb); // the new parents aabb is the leaf aabb combined with it's siblings aabb
			newParent.leftNodeIndex = leafSiblingIndex;
			newParent.rightNodeIndex = leafNodeIndex;
			leafNode.parentNodeIndex = newParentIndex;
			leafSibling.parentNodeIndex = newParentIndex;

			if (oldParentIndex == AABB_NULL_NODE)
			{
				// the old parent was the root and so this is now the root
				_rootNodeIndex = newParentIndex;
			}
			else
			{
				// the old parent was not the root and so we need to patch the left or right index to
				// point to the new node
				AABBNode<T>& oldParent = _nodes[oldParentIndex];
				if (oldParent.leftNodeIndex == leafSiblingIndex)
				{
					oldParent.leftNodeIndex = newParentIndex;
				}
				else
				{
					oldParent.rightNodeIndex = newParentIndex;
				}
			}

			// finally we need to walk back up the tree fixing heights and areas
			treeNodeIndex = leafNode.parentNodeIndex;
			FixUpwardsTree(treeNodeIndex);
		}

		void RemoveLeaf(unsigned leafNodeIndex)
		{
			// if the leaf is the root then we can just clear the root pointer and return
			if (leafNodeIndex == _rootNodeIndex)
			{
				_rootNodeIndex = AABB_NULL_NODE;
				return;
			}

			AABBNode<T>& leafNode = _nodes[leafNodeIndex];
			unsigned parentNodeIndex = leafNode.parentNodeIndex;
			const AABBNode<T>& parentNode = _nodes[parentNodeIndex];
			unsigned grandParentNodeIndex = parentNode.parentNodeIndex;
			unsigned siblingNodeIndex = parentNode.leftNodeIndex == leafNodeIndex ? parentNode.rightNodeIndex : parentNode.leftNodeIndex;
			assert(siblingNodeIndex != AABB_NULL_NODE); // we must have a sibling
			AABBNode<T>& siblingNode = _nodes[siblingNodeIndex];

			if (grandParentNodeIndex != AABB_NULL_NODE)
			{
				// if we have a grand parent (i.e. the parent is not the root) then destroy the parent and connect the sibling to the grandparent in its
				// place
				AABBNode<T>& grandParentNode = _nodes[grandParentNodeIndex];
				if (grandParentNode.leftNodeIndex == parentNodeIndex)
				{
					grandParentNode.leftNodeIndex = siblingNodeIndex;
				}
				else
				{
					grandParentNode.rightNodeIndex = siblingNodeIndex;
				}
				siblingNode.parentNodeIndex = grandParentNodeIndex;
				deallocateNode(parentNodeIndex);

				FixUpwardsTree(grandParentNodeIndex);
			}
			else
			{
				// if we have no grandparent then the parent is the root and so our sibling becomes the root and has it's parent removed
				_rootNodeIndex = siblingNodeIndex;
				siblingNode.parentNodeIndex = AABB_NULL_NODE;
				deallocateNode(parentNodeIndex);
			}

			leafNode.parentNodeIndex = AABB_NULL_NODE;
		}

		void UpdateLeaf(unsigned leafNodeIndex, const AABB& newAaab)
		{
			AABBNode<T>& node = _nodes[leafNodeIndex];

			// if the node contains the new aabb then we just leave things
			// TODO: when we add velocity this check should kick in as often an update will lie within the velocity fattened initial aabb
			// to support this we might need to differentiate between velocity fattened aabb and actual aabb
			if (node.aabb.Contains(newAaab)) return;

			RemoveLeaf(leafNodeIndex);
			node.aabb = newAaab;
			insertLeaf(leafNodeIndex);
		}


		void FixUpwardsTree(unsigned treeNodeIndex)
		{
			while (treeNodeIndex != AABB_NULL_NODE)
			{
				AABBNode<T>& treeNode = _nodes[treeNodeIndex];

				// every node should be a parent
				assert(treeNode.leftNodeIndex != AABB_NULL_NODE && treeNode.rightNodeIndex != AABB_NULL_NODE);

				// fix height and area
				const AABBNode<T>& leftNode = _nodes[treeNode.leftNodeIndex];
				const AABBNode<T>& rightNode = _nodes[treeNode.rightNodeIndex];
				treeNode.aabb = leftNode.aabb.Merge(rightNode.aabb);

				treeNodeIndex = treeNode.parentNodeIndex;
			}
		}
	};
}
