/*
The MIT License (MIT)
Copyright (c) 2016 James Yip
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef POOL_ALLOCATOR_H_
#define POOL_ALLOCATOR_H_

#include <cstddef>
#include <cstring>
#include <memory>

/// Block-based pool allocator for cache-friendly node allocation.
///
/// Allocates objects from contiguous memory blocks instead of individual
/// heap allocations. Freed slots are recycled via an intrusive freelist.
/// Designed for use with rp_heap's _Alloc template parameter.
///
/// Template parameters:
///   T         - element type
///   BlockSize - size of each memory block in bytes (default 4096)
template <class T, std::size_t BlockSize = 4096>
class pool_allocator
{
public:
    using value_type    = T;
    using pointer       = T*;
    using const_pointer = const T*;
    using reference     = T&;
    using const_reference = const T&;
    using size_type     = std::size_t;
    using difference_type = std::ptrdiff_t;

    template <class U>
    struct rebind { using other = pool_allocator<U, BlockSize>; };

private:
    // Alignment for each slot: must satisfy both T and pointer alignment
    static constexpr std::size_t slot_align =
        alignof(T) > alignof(void*) ? alignof(T) : alignof(void*);

    // Raw slot size: must hold either T or a freelist pointer
    static constexpr std::size_t raw_slot =
        sizeof(T) > sizeof(void*) ? sizeof(T) : sizeof(void*);

    // Padded slot size: rounded up to slot_align
    static constexpr std::size_t slot_size =
        ((raw_slot + slot_align - 1) / slot_align) * slot_align;

    // Header at start of each block stores the next-block pointer.
    // Rounded up to slot_align so slots start at an aligned address.
    static constexpr std::size_t header_size =
        ((sizeof(void*) + slot_align - 1) / slot_align) * slot_align;

    // Number of slots per block
    static constexpr std::size_t slots_per_block =
        (BlockSize > header_size) ? (BlockSize - header_size) / slot_size : 1;

    struct PoolState
    {
        char* block_list = nullptr;  // linked list of blocks; first bytes = next ptr
        char* free_list  = nullptr;  // freelist head; each slot stores next ptr

        void allocate_block()
        {
            std::size_t actual = header_size + slots_per_block * slot_size;
            char* block = static_cast<char*>(::operator new(actual));
            // Link new block to previous head
            std::memcpy(block, &block_list, sizeof(char*));
            block_list = block;
            // Slice block into slots and push onto freelist
            char* start = block + header_size;
            for (std::size_t i = 0; i < slots_per_block; ++i)
            {
                char* slot = start + i * slot_size;
                std::memcpy(slot, &free_list, sizeof(char*));
                free_list = slot;
            }
        }

        ~PoolState()
        {
            char* block = block_list;
            while (block)
            {
                char* next;
                std::memcpy(&next, block, sizeof(char*));
                ::operator delete(block);
                block = next;
            }
        }

        PoolState() = default;
        PoolState(const PoolState&) = delete;
        PoolState& operator=(const PoolState&) = delete;
    };

    std::shared_ptr<PoolState> state_;

public:
    pool_allocator() : state_(std::make_shared<PoolState>()) {}

    pool_allocator(const pool_allocator&) = default;
    pool_allocator& operator=(const pool_allocator&) = default;

    /// Rebind constructor: creates an independent pool for a different type.
    template <class U>
    pool_allocator(const pool_allocator<U, BlockSize>&)
        : state_(std::make_shared<PoolState>()) {}

    pointer allocate(size_type n)
    {
        if (n != 1)
            return static_cast<pointer>(::operator new(n * sizeof(T)));
        if (!state_->free_list)
            state_->allocate_block();
        char* slot = state_->free_list;
        std::memcpy(&state_->free_list, slot, sizeof(char*));
        return reinterpret_cast<pointer>(slot);
    }

    void deallocate(pointer p, size_type n)
    {
        if (n != 1)
        {
            ::operator delete(p);
            return;
        }
        char* slot = reinterpret_cast<char*>(p);
        std::memcpy(slot, &state_->free_list, sizeof(char*));
        state_->free_list = slot;
    }

    template <class U, class... Args>
    void construct(U* p, Args&&... args)
    {
        ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    template <class U>
    void destroy(U* p)
    {
        p->~U();
    }

    bool operator==(const pool_allocator& other) const
    {
        return state_ == other.state_;
    }

    bool operator!=(const pool_allocator& other) const
    {
        return !(*this == other);
    }
};

#endif /* POOL_ALLOCATOR_H_ */
