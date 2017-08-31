
#include <memory>
#include <vector>

namespace impl_fs_alloc {

	// One block
	template <size_t T_SIZE>
	struct Chunk
	{
		char data[T_SIZE];
		Chunk* next_chunk;
	};

	template <size_t T_SIZE, size_t ARRAY_SIZE>
	class Allocator
	{
		using chunk_t = Chunk<T_SIZE>;

		std::vector<std::unique_ptr<chunk_t[]>> chunks;
		chunk_t* pFreeChunk;
	public:
		static Allocator& instance() {
			static Allocator s_instance;
			return s_instance;
		}

		Allocator()
		{
			addChunks();
		}

		Allocator(const Allocator& clone) = delete;
		Allocator(Allocator&& clone) = delete;
		Allocator& operator=(const Allocator& clone) = delete;
		Allocator& operator=(Allocator&& clone) = delete;

		void addChunks()
		{
			chunk_t* array = new chunk_t[ARRAY_SIZE];
			chunks.emplace_back(array);

			for (auto i = 0; i < ARRAY_SIZE - 1; ++i)
				array[i].next_chunk = &array[i + 1];

			array[ARRAY_SIZE - 1].next_chunk = nullptr;
			pFreeChunk = array;
		}

		void* allocate()
		{
			if (pFreeChunk == nullptr)
				addChunks();

			chunk_t* pChunk = pFreeChunk;
			pFreeChunk = pChunk->next_chunk;

			return pChunk;
		}

		void deallocate(void* address)
		{
			auto pChunk = static_cast<chunk_t*>(address);

			pChunk->next_chunk = pFreeChunk;
			pFreeChunk = pChunk;
		}
	};
}



template<class T, size_t ARRAY_SIZE = 1024>
class FixedSizeAllocator {
public:
	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	template<typename U>
	struct rebind {
		typedef FixedSizeAllocator<U, ARRAY_SIZE> other;
	};
	using alloc_t = impl_fs_alloc::Allocator<sizeof(T), ARRAY_SIZE>;

	FixedSizeAllocator() = default;

	// allocation only for one block at the time

	pointer allocate(size_type nb) {
		
		if (nb == 1)
			return static_cast<pointer>(alloc_t::instance().allocate());
		else
			return std::allocator<T>().allocate(nb);
	}

	void deallocate(pointer pChunk, size_type nb) {
		
		if (nb == 1)
			alloc_t::instance().deallocate(static_cast<void*>(pChunk));
		else
			std::allocator<T>().deallocate(pChunk, nb);
	}

	void construct(pointer p) { new(p) T(); }
	void destroy(pointer p) { p->~T(); }
};

