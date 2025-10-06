#pragma once
#include <sys/types.h>
#include <utility>
#include <vector>

namespace kernel {
namespace concurrency {

template <class T>
class LockFreeQueue {
    struct Node {
        Node() : next(nullptr) {}
        T entity;
        Node* next;
    };
    public:
        explicit LockFreeQueue(size_t max_queue_size) :
            head_(new Node()), tail_(head_), size_(0UL),
            max_queue_size_(max_queue_size) {
        }

        ~LockFreeQueue() {
            volatile Node* e = head_;
            while (e != nullptr) {
                Node* tmp = e -> next;
                delete e;
                e = tmp;
            }
        }

        volatile const Node* Tail() {
            return tail_;
        }

        volatile const Node* Front() {
            return head_->next;
        }

        size_t Size() {
            return size_;
        }

        bool Empty() {
            return head_->next == nullptr;
        }

        bool Push(T&& entity) {
            size_t size = size_;
            if (size >= max_queue_size_ || !__sync_bool_compare_and_swap(&size_, size, size + 1UL)) {
                return false;
            }
            Node* node = new Node();
            node->entity = std::move(entity);
            volatile Node* tail = tail_;
            if (!__sync_bool_compare_and_swap(&tail_, tail, node)) {
                __sync_fetch_and_sub(&size_, 1UL);
                delete node;
                return false;
            }
            tail->next = node;
            return true;
        }

        bool Pop(T* entity) {
            size_t size = size_;
            if (size == 0UL || !__sync_bool_compare_and_swap(&size_, size, size - 1UL)) {
                return false;
            }
            volatile Node* head = head_;
            if (!__sync_bool_compare_and_swap(&head_, head, head->next == nullptr ? head_ : head->next) || head->next == nullptr) {
                __sync_fetch_and_add(&size_, 1UL);
                return false;
            }
            *entity = std::move(head->next->entity);
            delete head;
            return true;
        }

    private:
        volatile Node* head_;
        volatile Node* tail_;
        volatile size_t size_;
        size_t max_queue_size_;
};


}
}