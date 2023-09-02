/*
 * SeaLilyWalk 2023.8
 * 利用memory库中的allocator以栈的形式实现内存分配
 */

#ifndef ALLOCATOR_STACK_H
#define ALLOCATOR_STACK_H

#include <memory>

// 单向链表节点
template <typename T>
struct StackNode {
  T data;
  StackNode* prev;
  StackNode(): data(), prev(0){}
  StackNode(T elem, StackNode* p): data(elem), prev(p){}
};

// T是存储在栈中的对象类型
// Alloc是用来分配内存的allocator
template <class T, class Alloc = std::allocator<T> >
class AllocatorStack {
 public:
  AllocatorStack() {
    head_ = 0;
  }

  ~AllocatorStack() {
    clear();
  }

  bool is_empty() {
    return (head_ == 0);
  }

  // 清空栈
  void clear() {
    while (head_ != 0) {
      Node* prev = head_->prev;
      allocator_.destroy(head_);
      allocator_.deallocate(head_, 1);
      head_ = prev;
    }
  }

  void push(T elem) {
    Node* new_node = allocator_.allocate(1);
    allocator_.construct(new_node, Node(elem, head_));
    head_ = new_node;
  }

  T pop() {
    T result = head_->data;
    Node* tmp = head_->prev;
    allocator_.destroy(head_);
    allocator_.deallocate(head_, 1);
    head_ = tmp;
    return result;
  }

  T top() {
    return head_->data;
  }

 private:
  typedef StackNode<T> Node;
  typedef typename Alloc::template rebind<Node>::other allocator;
  allocator allocator_;
  Node* head_;
};

#endif
