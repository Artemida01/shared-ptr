#pragma once

#include "comn_inf.h"

template <typename T>
class shared_ptr {
public:
  template <typename>
  friend class weak_ptr;

  template <typename>
  friend class shared_ptr;

  using element_ptr = T*;
  using element_ref = T&;
  using block_ptr = base_block*;

public:
  shared_ptr() noexcept = default;

  explicit shared_ptr(std::nullptr_t) noexcept
      : shared_ptr() {}

  ~shared_ptr() {
    if (_control_block) {
      _control_block->del_real();
    }
  }

  template <typename Y>
  explicit shared_ptr(Y* ptr)
      : shared_ptr(ptr, std::default_delete<Y>{}) {}

  template <typename Y, typename Deleter>
  shared_ptr(Y* ptr, Deleter deleter)
      : _element(ptr) {
    try {
      _control_block = new derived_block_element(ptr, std::move(deleter));
    } catch (...) {
      std::invoke(deleter, ptr);
      throw;
    }
  }

  template <typename Y>
  shared_ptr(const shared_ptr<Y>& other, element_ptr ptr) noexcept
      : shared_ptr(ptr, other._control_block) {}

  template <typename Y>
  shared_ptr(shared_ptr<Y>&& other, element_ptr ptr) noexcept
      : _element(ptr)
      , _control_block(std::exchange(other._control_block, nullptr)) {
    std::exchange(other._element, nullptr);
  }

  shared_ptr(const shared_ptr& other) noexcept
      : shared_ptr(other._element, other._control_block) {}

  template <typename Y>
    requires (std::is_convertible_v<Y*, T*>)
  shared_ptr(const shared_ptr<Y>& other) noexcept
      : shared_ptr(other._element, other._control_block) {}

  shared_ptr(shared_ptr&& other) noexcept
      : _element(std::exchange(other._element, nullptr))
      , _control_block(std::exchange(other._control_block, nullptr)) {}

  template <typename Y>
  shared_ptr(shared_ptr<Y>&& other) noexcept
      : _element(std::exchange(other._element, nullptr))
      , _control_block(std::exchange(other._control_block, nullptr)) {}

  shared_ptr& operator=(const shared_ptr& other) noexcept {
    if (this != &other) {
      shared_ptr(other).swap(*this);
    }
    return *this;
  }

  template <typename Y>
  shared_ptr& operator=(const shared_ptr<Y>& other) noexcept {
    shared_ptr(other).swap(*this);
    return *this;
  }

  shared_ptr& operator=(shared_ptr&& other) noexcept {
    if (this != &other) {
      shared_ptr(std::move(other)).swap(*this);
    }
    return *this;
  }

  template <typename Y>
  shared_ptr& operator=(shared_ptr<Y>&& other) noexcept {
    shared_ptr(std::move(other)).swap(*this);
    return *this;
  }

  element_ptr get() const noexcept {
    return _element;
  }

  explicit operator bool() const noexcept {
    return (_element != nullptr);
  }

  element_ref operator*() const noexcept {
    return *get();
  }

  element_ptr operator->() const noexcept {
    return get();
  }

  std::size_t use_count() const noexcept {
    if (_control_block) {
      return _control_block->get_real();
    }
    return 0;
  }

  void reset() noexcept {
    shared_ptr().swap(*this);
  }

  template <typename Y>
  void reset(Y* new_ptr) {
    shared_ptr(new_ptr).swap(*this);
  }

  template <typename Y, typename Deleter>
  void reset(Y* new_ptr, Deleter deleter) {
    shared_ptr<T>(new_ptr, std::move(deleter)).swap(*this);
  }

  void swap(shared_ptr& other) noexcept {
    std::swap(other._element, _element);
    std::swap(other._control_block, _control_block);
  }

  template <typename Y>
    requires (std::is_convertible_v<Y*, T*>)
  explicit shared_ptr(Y* ptr, block_ptr block)
      : _element(ptr)
      , _control_block(block) {
    if (_control_block) {
      _control_block->new_real();
    }
  }

  explicit shared_ptr(element_ptr ptr, derived_block_full<T>* block)
      : _element(ptr)
      , _control_block(block) {}

private:
  element_ptr _element{nullptr};
  block_ptr _control_block{nullptr};
};

template <typename LType, typename RType>
bool operator==(const shared_ptr<LType>& lhs, const shared_ptr<RType>& rhs) noexcept {
  return (lhs.get() == rhs.get());
}

template <typename T>
bool operator==(std::nullptr_t, const shared_ptr<T>& rhs) noexcept {
  return !static_cast<bool>(rhs);
}

template <typename T>
bool operator==(const shared_ptr<T>& lhs, std::nullptr_t) noexcept {
  return !static_cast<bool>(lhs);
}

template <typename LType, typename RType>
bool operator!=(const shared_ptr<LType>& lhs, const shared_ptr<RType>& rhs) noexcept {
  return !(lhs == rhs);
}

template <typename T>
class weak_ptr {
public:
  using element_ptr = T*;
  using element_ref = T&;
  using block_ptr = base_block*;

  template <typename>
  friend class weak_ptr;

public:
  weak_ptr() noexcept = default;

  ~weak_ptr() {
    if (_control_block) {
      _control_block->del_weak();
    }
  }

  template <typename Y>
  weak_ptr(const shared_ptr<Y>& other) noexcept
      : weak_ptr(other._element, other._control_block) {}

  weak_ptr(const weak_ptr& other) noexcept
      : weak_ptr(other._element, other._control_block) {}

  template <typename Y>
  weak_ptr(const weak_ptr<Y>& other) noexcept
      : weak_ptr(other._element, other._control_block) {}

  weak_ptr(weak_ptr&& other) noexcept
      : _element(std::exchange(other._element, nullptr))
      , _control_block(std::exchange(other._control_block, nullptr)) {}

  template <typename Y>
    requires (std::is_convertible_v<Y*, T*>)
  weak_ptr(weak_ptr<Y>&& other) noexcept
      : _element(std::exchange(other._element, nullptr))
      , _control_block(std::exchange(other._control_block, nullptr)) {}

  template <typename Y>
  weak_ptr& operator=(const shared_ptr<Y>& other) noexcept {
    weak_ptr(other).swap(*this);
    return *this;
  }

  weak_ptr& operator=(const weak_ptr& other) noexcept {
    if (this != &other) {
      weak_ptr(other).swap(*this);
    }
    return *this;
  }

  template <typename Y>
  weak_ptr& operator=(const weak_ptr<Y>& other) noexcept {
    weak_ptr(other).swap(*this);
    return *this;
  }

  weak_ptr& operator=(weak_ptr&& other) noexcept {
    if (this != &other) {
      weak_ptr(std::move(other)).swap(*this);
    }
    return *this;
  }

  template <typename Y>
  weak_ptr& operator=(weak_ptr<Y>&& other) noexcept {
    weak_ptr(std::move(other)).swap(*this);
    return *this;
  }

  shared_ptr<T> lock() const noexcept {
    if (_control_block == nullptr || _control_block->get_real() == 0) {
      return shared_ptr<T>();
    }
    return shared_ptr<T>(_element, _control_block);
  }

  void reset() noexcept {
    weak_ptr().swap(*this);
  }

  void swap(weak_ptr& other) noexcept {
    using std::swap;
    swap(_element, other._element);
    swap(_control_block, other._control_block);
  }

  template <typename Y>
  explicit weak_ptr(Y* other_elem, block_ptr other_block) noexcept
      : _element(other_elem)
      , _control_block(other_block) {
    if (_control_block) {
      _control_block->new_weak();
    }
  }

private:
  element_ptr _element{nullptr};
  block_ptr _control_block{nullptr};
};

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
  auto* ptr = new derived_block_full<T>(std::forward<Args>(args)...);
  return shared_ptr<T>(ptr->get_element(), ptr);
}
