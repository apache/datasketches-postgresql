/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
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

  void deallocate(pointer p, size_type) { if (p) pfree(p); }

  size_type max_size() const {
    return static_cast<size_type>(-1) / sizeof(T);
  }

  template<typename... Args>
  void construct(pointer p, Args&&... args) {
    new(p) value_type(std::forward<Args>(args)...);
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
