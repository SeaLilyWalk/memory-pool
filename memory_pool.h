#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <iostream>
#include <cstddef>

template <typename T, size_t BlockSize = 4096>
class MemoryPool {
 public:
  template <typename U> struct rebind {
    typedef MemoryPool<U> other;
  };

  MemoryPool() noexcept;
  MemoryPool(const MemoryPool& memory_pool) noexcept;
  MemoryPool(MemoryPool&& memory_pool) noexcept;
  template <class U> MemoryPool(const MemoryPool<U>& memory_pool) noexcept;

  ~MemoryPool() noexcept;

  MemoryPool& operator=(const MemoryPool& memoryPool) = delete;
  MemoryPool& operator=(MemoryPool&& memoryPool) noexcept;

  T* address(T& x) const noexcept;
  const T* address(const T& x) const noexcept;

  // 每一次只能allocate一个对象，参数n将会被忽视
  T* allocate(size_t n = 1, const T* hint = 0);
  void deallocate(T* p, size_t n = 1);

  size_t max_size() const noexcept;

  template <class U, class... Args> void construct(U* p, Args&&... args);
  template <class U> void destroy(U* p);
  template <class... Args> T* newElement(Args&&... args);
  void deleteElement(T* p);

 private:
  union Slot_ {
    T elem;
    Slot_* nxt;
  };

  Slot_* current_block_;
  Slot_* current_slot_;
  Slot_* last_slot_;
  Slot_* free_slots_;

  size_t padPointer(char* p, size_t align) const noexcept;
  void allocateBlock();
  
  static_assert(BlockSize >= 2*sizeof(Slot_), "BlockSize too small");
};

// 貌似是用于计算对齐的
template <typename T, size_t BlockSize>
size_t MemoryPool<T, BlockSize>::padPointer(char* p, size_t align)
const noexcept {
  uintptr_t result = reinterpret_cast<uintptr_t>(p);
  return ((align - result)%align);
}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool() noexcept {
  current_block_ = nullptr;
  current_slot_ = nullptr;
  last_slot_ = nullptr;
  free_slots_ = nullptr;
}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool& memoryPool) noexcept: MemoryPool() {}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(MemoryPool&& memoryPool) noexcept {
  current_block_ = memoryPool.current_block_;
  memoryPool.current_block_ = nullptr;
  current_slot_ = memoryPool.current_slot_;
  last_slot_ = memoryPool.last_slot_;
  free_slots_ = memoryPool.free_slots_;
}


template <typename T, size_t BlockSize>
template <class U>
MemoryPool <T, BlockSize>::MemoryPool(const MemoryPool<U>& memoryPool) noexcept:
MemoryPool() {}


template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>&
MemoryPool<T, BlockSize>::operator=(MemoryPool&& memoryPool) noexcept {
  if (this != &memoryPool) {
    std::swap(current_block_, memoryPool.current_block_);
    current_slot_ = memoryPool.current_slot_;
    last_slot_ = memoryPool.last_slot_;
    free_slots_ = memoryPool.free_slots_;
  }
  return *this;
}


template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool() noexcept {
  Slot_* curr = current_block_;
  while (curr != nullptr) {
    Slot_* prev = curr->nxt;
    operator delete(reinterpret_cast<void*>(curr));
    curr = prev;
  }
}

template <typename T, size_t BlockSize>
T* MemoryPool<T, BlockSize>::address(T& x) const noexcept {
  return &x;
}

template <typename T, size_t BlockSize>
const T* MemoryPool<T, BlockSize>::address(const T& x) const noexcept {
  return &x;
}

template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::allocateBlock() {
  char* new_block = reinterpret_cast<char*> (operator new(BlockSize));
  reinterpret_cast<Slot_*>(new_block)->nxt = current_block_;
  current_block_ = reinterpret_cast<Slot_*>(new_block);
  char* body = new_block + sizeof(Slot_*);
  size_t body_padding = padPointer(body, alignof(Slot_));
  current_slot_ = reinterpret_cast<Slot_*>(body + body_padding);
  last_slot_ = reinterpret_cast<Slot_*> (new_block + BlockSize - sizeof(Slot_) + 1);
}

template <typename T, size_t BlockSize>
T* MemoryPool<T, BlockSize>::allocate(size_t n, const T* hint) {
  if (free_slots_ != nullptr) {
    T* result = reinterpret_cast<T*>(free_slots_);
    free_slots_ = free_slots_->nxt;
    return result;
  }
  else {
    if (current_slot_ >= last_slot_)
      allocateBlock();
    return reinterpret_cast<T*>(current_slot_++);
  }
}

template <typename T, size_t BlockSize>
inline void MemoryPool<T, BlockSize>::deallocate(T* p, size_t n) {
  if (p != nullptr) {
    reinterpret_cast<Slot_*>(p)->nxt = free_slots_;
    free_slots_ = reinterpret_cast<Slot_*>(p);
  }
}


template <typename T, size_t BlockSize>
inline size_t MemoryPool<T, BlockSize>::max_size() const noexcept {
  size_t max_blocks = -1/BlockSize;
  size_t result = (BlockSize - sizeof(char*)) / sizeof(Slot_) * max_blocks;
  return result;
}

template <typename T, size_t BlockSize>
template <class U, class... Args>
inline void MemoryPool<T, BlockSize>::construct(U* p, Args&&... args) {
  new (p) U (std::forward<Args>(args)...);
}

template <typename T, size_t BlockSize>
template <class U> inline void MemoryPool<T, BlockSize>::destroy(U* p) {
  p->~U();
}

template <typename T, size_t BlockSize>
template <class... Args> inline T* MemoryPool<T, BlockSize>::newElement(Args&&... args) {
  T* result = allocate();
  construct<T>(result, std::forward<Args>(args)...);
  return result;
}

template <typename T, size_t BlockSize>
inline void MemoryPool<T, BlockSize>::deleteElement(T* p) {
  if (p != nullptr) {
    p->~T();
    deallocate(p);
  }
}

#endif
