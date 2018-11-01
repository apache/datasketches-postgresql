/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

extern "C" {
#include <postgres.h>
}

#include <new>
#include <utility>

template <class T> class palloc_allocator {
public:
  typedef T                 value_type;
  typedef value_type*       pointer;
  typedef const value_type* const_pointer;
  typedef value_type&       reference;
  typedef const value_type& const_reference;
  typedef std::size_t       size_type;
  typedef std::ptrdiff_t    difference_type;

  template <class U>
  struct rebind { typedef palloc_allocator<U> other; };

  palloc_allocator() {}
  palloc_allocator(const palloc_allocator&) {}
  template <class U>
  palloc_allocator(const palloc_allocator<U>&) {}
  ~palloc_allocator() {}

  pointer address(reference x) const { return &x; }
  const_pointer address(const_reference x) const {
    return x;
  }

  pointer allocate(size_type n, const_pointer = 0) {
    void* p = palloc(n * sizeof(T));
    if (!p) throw std::bad_alloc();
    return static_cast<pointer>(p);
  }

  void deallocate(pointer p, size_type) { pfree(p); }

  size_type max_size() const {
    return static_cast<size_type>(-1) / sizeof(T);
  }

  void construct(pointer p, const value_type&& x) {
    new(p) value_type(std::forward<const value_type>(x));
  }
  void destroy(pointer p) { p->~value_type(); }

private:
  void operator=(const palloc_allocator&);
};

template<> class palloc_allocator<void> {
public:
  typedef void        value_type;
  typedef void*       pointer;
  typedef const void* const_pointer;

  template <class U>
  struct rebind { typedef palloc_allocator<U> other; };
};


template <class T>
inline bool operator==(const palloc_allocator<T>&, const palloc_allocator<T>&) {
  return true;
}

template <class T>
inline bool operator!=(const palloc_allocator<T>&, const palloc_allocator<T>&) {
  return false;
}
