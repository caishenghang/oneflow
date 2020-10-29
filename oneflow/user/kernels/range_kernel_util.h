/*
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef ONEFLOW_USER_KERNELS_RANGE_KERNEL_UTIL_H_
#define ONEFLOW_USER_KERNELS_RANGE_KERNEL_UTIL_H_

#include "oneflow/core/device/device_context.h"

namespace oneflow {

template<DeviceType device_type, typename T>
struct RangeKernelUtil {
  static void Range(DeviceCtx* ctx, const int start, const int delta, const int range_shape,
                    T* out);
};

#define INSTANTIATE_RANGE_FUNCTOR(device_type_v, dtype_pair) \
  template struct RangeKernelUtil<device_type_v, OF_PP_PAIR_FIRST(dtype_pair)>;

}  // namespace oneflow

#endif  // ONEFLOW_USER_KERNELS_RANGE_KERNEL_UTIL_H_
