// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef  BASE_MEMORY_UTILS_HPP_
#define  BASE_MEMORY_UTILS_HPP_

#include <memory>
#include "base/memory_utils_base.hpp"

//////////////////////////////////////// pointer/handle convert ////////////////////////////////////////

inline uintptr_t handle_from_pointer(void* raw_poionter) { return reinterpret_cast<uintptr_t>(raw_poionter); }
template<typename T> inline uintptr_t handle_from_pointer(std::shared_ptr<T> sp_pointer) { return handle_from_pointer(sp_pointer.get()); }
template<typename T> inline uintptr_t handle_from_pointer(std::weak_ptr<T> wsp_pointer) { return handle_from_pointer(wsp_pointer.lock()); }
template<typename T> inline uintptr_t handle_from_pointer(std::unique_ptr<T> sp_pointer) { return handle_from_pointer(sp_pointer.get()); }
template<typename T> inline T* handle_to_pointer(uintptr_t handle) { return reinterpret_cast<T*>(handle); }

inline uintptr_t object_handle_from_pointer(virtual_enable_shared_from_this_base* raw_poionter) { return handle_from_pointer(raw_poionter); }
inline uintptr_t object_handle_from_pointer(std::shared_ptr<virtual_enable_shared_from_this_base> sp_pointer) { return object_handle_from_pointer(sp_pointer.get()); }
inline uintptr_t object_handle_from_pointer(std::weak_ptr<virtual_enable_shared_from_this_base> wsp_pointer) { return object_handle_from_pointer(wsp_pointer.lock()); }
inline uintptr_t object_handle_from_pointer(std::unique_ptr<virtual_enable_shared_from_this_base> sp_pointer) { return object_handle_from_pointer(sp_pointer.get()); }
inline virtual_enable_shared_from_this_base* object_handle_to_raw_pointer(uintptr_t handle) { return handle_to_pointer<virtual_enable_shared_from_this_base>(handle); }
inline std::shared_ptr<virtual_enable_shared_from_this_base> object_handle_to_pointer(uintptr_t handle) { return object_handle_to_raw_pointer(handle)->shared_from_this(); }
inline std::weak_ptr<virtual_enable_shared_from_this_base> object_handle_to_weak_pointer(uintptr_t handle) { return object_handle_to_pointer(handle); }

template<typename T> inline virtual_enable_shared_from_this_base* object_raw_pointer_from(T* p_this) { return p_this; }
template<typename T> inline std::shared_ptr<virtual_enable_shared_from_this_base> object_pointer_from(T* p_this) { return object_raw_pointer_from(p_this)->shared_from_this(); }
template<typename T> inline std::weak_ptr<virtual_enable_shared_from_this_base> object_weak_pointer_from(T* p_this) { return object_pointer_from(p_this); }

#endif  // BASE_MEMORY_UTILS_HPP_
