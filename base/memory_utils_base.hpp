// Copyright (c) 2022 The csew Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//
// reference:
//  1. https://stackoverflow.com/questions/14939190/boost-shared-from-this-and-multiple-inheritance
//  1. https://stackoverflow.com/questions/15549722/double-inheritance-of-enable-shared-from-this
//  1. https://stackoverflow.com/questions/16082785/use-of-enable-shared-from-this-with-multiple-inheritance
//  1. https://gist.github.com/zhangyuchi/973f39d02460f3b597bd
//

#ifndef  BASE_MEMORY_UTILS_BASE_HPP_
#define  BASE_MEMORY_UTILS_BASE_HPP_

#include <memory>

class virtual_enable_shared_from_this_base : public std::enable_shared_from_this<virtual_enable_shared_from_this_base> {
 public:
    virtual ~virtual_enable_shared_from_this_base(void) {}
};

template <class T>
class virtual_enable_shared_from_this : virtual public virtual_enable_shared_from_this_base {
 public:
    std::shared_ptr<T> shared_from_this() { return std::dynamic_pointer_cast<T>(virtual_enable_shared_from_this_base::shared_from_this()); }
    /* Utility method to easily downcast.
     * Useful when a child doesn't inherit directly from enable_shared_from_this
     * but wants to use the feature.
     */
    template <class Down> std::shared_ptr<Down> downcasted_shared_from_this(void) {
        return std::dynamic_pointer_cast<Down>(virtual_enable_shared_from_this_base::shared_from_this());
    }
};

#endif  // BASE_MEMORY_UTILS_BASE_HPP_
