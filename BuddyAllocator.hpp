#pragma once

#include <cstddef>
#include <stdexcept>
#include <bitset>

constexpr static auto MAX = 30;
constexpr static auto MIN = 9;
constexpr static auto MAX_BLOCK = 1 << MAX;
constexpr static auto MIN_BLOCK = 1 << MIN;
constexpr static auto LEVELS = (MAX - MIN) + 1;

class BuddyAllocator
{
private:
	std::byte* mem;
	struct Node
	{
		Node* prev;
		Node* next;
	};
private:
	Node* firstFree[LEVELS] = {nullptr};
	std::bitset<(1 << LEVELS) - 1> freeBits = {true};
public:
	BuddyAllocator()
	{
		this->mem = new std::byte[MAX_BLOCK]();
		firstFree[0] = reinterpret_cast<Node*>(mem); // Its prev and next are auto nullptr
	};
	~BuddyAllocator()
	{
		delete[] this->mem;
	};

	[[nodiscard]]
	void* alloc(size_t size)
	{
		auto lvl = levelFromSize(size);
		Node* node = getNodeAtLevel(lvl);
		if (!node)
			return nullptr;

		firstFree[lvl] = node->next;
		return reinterpret_cast<void*>(node);
	};

	void free(void* ptr, size_t size)
	{
		if (!ptr) return; // freeing nullptr

		int lvl = levelFromSize(size);

		Node* freed = (Node*)ptr;
		if (firstFree[lvl]) firstFree[lvl]->prev = freed;
		freed[0] = Node{nullptr, firstFree[lvl] == freed ? nullptr : firstFree[lvl]};
		firstFree[lvl] = freed;

		mergeNode(freed, lvl);
	};

	void reset()
	{
		freeBits[0] = true;

		for (auto& n : firstFree) n = nullptr;
		firstFree[0] = reinterpret_cast<Node*>(mem);

	}
private:
	static size_t pow2Size(size_t size)
	{
		size_t ret = MIN_BLOCK;
		for (; ret < size; ret *= 2);
		return ret;
	}
	static int levelFromSize(size_t size)
	{
		if (size <= MIN_BLOCK) return LEVELS - 1;
		size = pow2Size(size);
		size >>= 9; // size /= 512
		int level;
		for (level = LEVELS - 1; size > 1; --level)
			size >>= 1;
		return level;
	}

	static size_t sizeOfLevel(int level)
	{
		return MAX_BLOCK / (1 << level);
	}

	/// Splits nodes until a node is available at the requested level
	Node* getNodeAtLevel(int level)
	{
		if (this->firstFree[level])
		{
			Node* retNode = this->firstFree[level];
			freeBits[indexOf(retNode, level)] = false;
			return retNode;
		}
		if (level == 0)
			return nullptr;
		//throw std::runtime_error("Not enough space to allocate");

	// We have to split
		Node* splitNode = getNodeAtLevel(level - 1);
		if (!splitNode)
			return nullptr;

		firstFree[level - 1] = splitNode->next;
		Node* first = splitNode;
		Node* second = (Node*)((std::byte*)first + sizeOfLevel(level));
		first[0] = Node{nullptr, second};
		second[0] = Node{nullptr, nullptr};
		freeBits[indexOf(second, level)] = true;
		firstFree[level] = first;
		return first;
	}

	size_t indexOf(Node* node, int level) const
	{
		size_t inLvl = ((std::byte*)node - this->mem) / sizeOfLevel(level);
		return ((1 << level) - 1) + inLvl;
	}

	void mergeNode(Node* node, int level)
	{
		const size_t index = indexOf(node, level);
		freeBits[index] = true;
		if (level == 0)
		{
			return;
		}

		const size_t buddyIndex = (index % 2 == 0) ? index - 1 : index + 1;
		if (freeBits[buddyIndex])
		{
			const size_t relativePtr = (buddyIndex - ((1 << level) - 1)) * sizeOfLevel(level);
			Node* buddy = (Node*)(this->mem + relativePtr);
			while (firstFree[level] == node || firstFree[level] == buddy)
			{
				firstFree[level] = firstFree[level]->next;
				if (firstFree[level]) firstFree[level]->prev = nullptr;
			}
			if (buddy->prev) buddy->prev->next = buddy->next;
			if (buddy->next) buddy->next->prev = buddy->prev;
			if (node->prev) node->prev->next = node->next;
			if (node->next) node->next->prev = node->prev;

			Node* parent = node < buddy ? node : buddy;

			if (firstFree[level - 1]) firstFree[level - 1]->prev = parent;
			parent[0] = Node{nullptr, firstFree[level - 1]};
			firstFree[level - 1] = parent;

			mergeNode(parent, level - 1);
		}
	}
};
