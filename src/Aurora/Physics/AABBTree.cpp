#include "AABBTree.hpp"

namespace Aurora
{
	AABBTreeRaw::AABBTreeRaw(unsigned initialSize) : _rootNodeIndex(AABB_NULL_NODE), _allocatedNodeCount(0), _nextFreeNodeIndex(0), _nodeCapacity(initialSize), _growthSize(initialSize)
	{
		_nodes.resize(initialSize);
		for (unsigned nodeIndex = 0; nodeIndex < initialSize; nodeIndex++)
		{
			AABBNodeRaw& node = _nodes[nodeIndex];
			node.nextNodeIndex = nodeIndex + 1;
		}
		_nodes[initialSize-1].nextNodeIndex = AABB_NULL_NODE;
	}

	AABBTreeRaw::~AABBTreeRaw()
	{
	}

	unsigned AABBTreeRaw::allocateNode()
	{
		// if we have no free tree nodes then grow the pool
		if (_nextFreeNodeIndex == AABB_NULL_NODE)
		{
			assert(_allocatedNodeCount == _nodeCapacity);

			_nodeCapacity += _growthSize;
			_nodes.resize(_nodeCapacity);
			for (unsigned nodeIndex = _allocatedNodeCount; nodeIndex < _nodeCapacity; nodeIndex++)
			{
				AABBNodeRaw& node = _nodes[nodeIndex];
				node.nextNodeIndex = nodeIndex + 1;
			}
			_nodes[_nodeCapacity - 1].nextNodeIndex = AABB_NULL_NODE;
			_nextFreeNodeIndex = _allocatedNodeCount;
		}

		unsigned nodeIndex = _nextFreeNodeIndex;
		AABBNodeRaw& allocatedNode = _nodes[nodeIndex];
		allocatedNode.parentNodeIndex = AABB_NULL_NODE;
		allocatedNode.leftNodeIndex = AABB_NULL_NODE;
		allocatedNode.rightNodeIndex = AABB_NULL_NODE;
		_nextFreeNodeIndex = allocatedNode.nextNodeIndex;
		_allocatedNodeCount++;

		return nodeIndex;
	}

	void AABBTreeRaw::deallocateNode(unsigned nodeIndex)
	{
		AABBNodeRaw& deallocatedNode = _nodes[nodeIndex];
		deallocatedNode.nextNodeIndex = _nextFreeNodeIndex;
		_nextFreeNodeIndex = nodeIndex;
		_allocatedNodeCount--;
	}

	void AABBTreeRaw::insertObject(ColliderComponent* object, const AABB& aabb)
	{
		unsigned nodeIndex = allocateNode();
		AABBNodeRaw& node = _nodes[nodeIndex];

		node.aabb = aabb;
		node.object = object;

		insertLeaf(nodeIndex);
		_objectNodeIndexMap[object] = nodeIndex;
	}

	void AABBTreeRaw::removeObject(ColliderComponent* object)
	{
		unsigned nodeIndex = _objectNodeIndexMap[object];
		removeLeaf(nodeIndex);
		deallocateNode(nodeIndex);
		_objectNodeIndexMap.erase(object);
	}

	void AABBTreeRaw::updateObject(ColliderComponent* object, const AABB& aabb)
	{
		unsigned nodeIndex = _objectNodeIndexMap[object];
		updateLeaf(nodeIndex, aabb);
	}

	std::forward_list<ColliderComponent*> AABBTreeRaw::queryOverlaps(ColliderComponent* object, const AABB& aabb) const
	{
		std::forward_list<ColliderComponent*> overlaps;
		std::stack<unsigned> stack;
		AABB testAabb = aabb;

		stack.push(_rootNodeIndex);
		while(!stack.empty())
		{
			unsigned nodeIndex = stack.top();
			stack.pop();

			if (nodeIndex == AABB_NULL_NODE) continue;

			const AABBNodeRaw& node = _nodes[nodeIndex];
			if (node.aabb.Overlaps(testAabb))
			{
				if (node.isLeaf() && node.object != object)
				{
					overlaps.push_front(node.object);
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

	void AABBTreeRaw::insertLeaf(unsigned leafNodeIndex)
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
		AABBNodeRaw& leafNode = _nodes[leafNodeIndex];
		while (!_nodes[treeNodeIndex].isLeaf())
		{
			// because of the test in the while loop above we know we are never a leaf inside it
			const AABBNodeRaw& treeNode = _nodes[treeNodeIndex];
			unsigned leftNodeIndex = treeNode.leftNodeIndex;
			unsigned rightNodeIndex = treeNode.rightNodeIndex;
			const AABBNodeRaw& leftNode = _nodes[leftNodeIndex];
			const AABBNodeRaw& rightNode = _nodes[rightNodeIndex];

			AABB combinedAabb = treeNode.aabb.Merge(leafNode.aabb);

			float newParentNodeCost = 2.0f * combinedAabb.GetSurfaceArea();
			float minimumPushDownCost = 2.0f * (combinedAabb.GetSurfaceArea() - treeNode.aabb.GetSurfaceArea());

			// use the costs to figure out whether to create a new parent here or descend
			float costLeft;
			float costRight;
			if (leftNode.isLeaf())
			{
				costLeft = leafNode.aabb.Merge(leftNode.aabb).GetSurfaceArea() + minimumPushDownCost;
			}
			else
			{
				AABB newLeftAabb = leafNode.aabb.Merge(leftNode.aabb);
				costLeft = (newLeftAabb.GetSurfaceArea() - leftNode.aabb.GetSurfaceArea()) + minimumPushDownCost;
			}
			if (rightNode.isLeaf())
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
		AABBNodeRaw& leafSibling = _nodes[leafSiblingIndex];
		unsigned oldParentIndex = leafSibling.parentNodeIndex;
		unsigned newParentIndex = allocateNode();
		AABBNodeRaw& newParent = _nodes[newParentIndex];
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
			AABBNodeRaw& oldParent = _nodes[oldParentIndex];
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
		fixUpwardsTree(treeNodeIndex);
	}

	void AABBTreeRaw::removeLeaf(unsigned leafNodeIndex)
	{
		// if the leaf is the root then we can just clear the root pointer and return
		if (leafNodeIndex == _rootNodeIndex)
		{
			_rootNodeIndex = AABB_NULL_NODE;
			return;
		}

		AABBNodeRaw& leafNode = _nodes[leafNodeIndex];
		unsigned parentNodeIndex = leafNode.parentNodeIndex;
		const AABBNodeRaw& parentNode = _nodes[parentNodeIndex];
		unsigned grandParentNodeIndex = parentNode.parentNodeIndex;
		unsigned siblingNodeIndex = parentNode.leftNodeIndex == leafNodeIndex ? parentNode.rightNodeIndex : parentNode.leftNodeIndex;
		assert(siblingNodeIndex != AABB_NULL_NODE); // we must have a sibling
		AABBNodeRaw& siblingNode = _nodes[siblingNodeIndex];

		if (grandParentNodeIndex != AABB_NULL_NODE)
		{
			// if we have a grand parent (i.e. the parent is not the root) then destroy the parent and connect the sibling to the grandparent in its
			// place
			AABBNodeRaw& grandParentNode = _nodes[grandParentNodeIndex];
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

			fixUpwardsTree(grandParentNodeIndex);
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

	void AABBTreeRaw::updateLeaf(unsigned leafNodeIndex, const AABB& newAaab)
	{
		AABBNodeRaw& node = _nodes[leafNodeIndex];

		// if the node contains the new aabb then we just leave things
		// TODO: when we add velocity this check should kick in as often an update will lie within the velocity fattened initial aabb
		// to support this we might need to differentiate between velocity fattened aabb and actual aabb
		if (node.aabb.Contains(newAaab)) return;

		removeLeaf(leafNodeIndex);
		node.aabb = newAaab;
		insertLeaf(leafNodeIndex);
	}


	void AABBTreeRaw::fixUpwardsTree(unsigned treeNodeIndex)
	{
		while (treeNodeIndex != AABB_NULL_NODE)
		{
			AABBNodeRaw& treeNode = _nodes[treeNodeIndex];

			// every node should be a parent
			assert(treeNode.leftNodeIndex != AABB_NULL_NODE && treeNode.rightNodeIndex != AABB_NULL_NODE);

			// fix height and area
			const AABBNodeRaw& leftNode = _nodes[treeNode.leftNodeIndex];
			const AABBNodeRaw& rightNode = _nodes[treeNode.rightNodeIndex];
			treeNode.aabb = leftNode.aabb.Merge(rightNode.aabb);

			treeNodeIndex = treeNode.parentNodeIndex;
		}
	}
}