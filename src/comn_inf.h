#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

struct base_block {
public:
  base_block() = default;

  virtual ~base_block() = default;

  void new_real() noexcept {
    real_link++;
  }

  void del_real() noexcept {
    if (--real_link == 0) {
      erase();
      if (get_weak() == 0) {
        delete this;
      }
    }
  }

  void new_weak() noexcept {
    weak_link++;
  }

  void del_weak() noexcept {
    if (--weak_link == 0 && get_real() == 0) {
      delete this;
    }
  }

  [[nodiscard]] std::size_t get_weak() const noexcept {
    return weak_link;
  }

  [[nodiscard]] std::size_t get_real() const noexcept {
    return real_link;
  }

protected:
  virtual void erase() = 0;

private:
  std::size_t weak_link{0};
  std::size_t real_link{1};
};

template <typename T, typename Deleter = std::default_delete<T>>
struct derived_block_element : base_block {
public:
  void erase() override {
    assert(get_real() == 0);
    _deleter(std::exchange(_element, nullptr));
  }

  ~derived_block_element() override = default;

  derived_block_element() = default;

  explicit derived_block_element(T* other_elem, Deleter deleter)
      : _element(other_elem)
      , _deleter(std::move(deleter)) {}

private:
  T* _element;
  Deleter _deleter;
};

template <typename T>
struct derived_block_full : base_block {
public:
  void erase() override {
    assert(get_real() == 0);
    reinterpret_cast<T*>(&_element)->~T(); // std::destroy_at(_element);
  }

  ~derived_block_full() override {} // {}

  derived_block_full() = default;

  template <typename... Args>
  explicit derived_block_full(Args&&... args)
      : _element(std::forward<Args>(args)...) {}

  T* get_element() {
    return &_element;
  }

private:
  union {
    T _element;
  };
};
