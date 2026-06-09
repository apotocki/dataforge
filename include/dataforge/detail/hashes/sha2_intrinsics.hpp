/*=============================================================================
    Copyright (c) Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <cstdint>

namespace dataforge::sha2_detail {

#if DATAFORGE_SHA256_ACCEL_CAN_COMPILE_X86_SHA

inline void process_block_sha256_x86(uint32_t state[8], const void* msg);

#endif

#if DATAFORGE_SHA256_ACCEL_CAN_COMPILE_ARM_SHA2

inline void process_block_sha256_arm(uint32_t state[8], const void* msg);

#endif

#if DATAFORGE_SHA256_ACCEL_IMPL == DATAFORGE_SHA256_ACCEL_AUTODETECT_MODE

inline bool sha256_runtime_has_sha256_accel();

#endif

}

#include "sha2_intrinsics_x86.ipp"
#include "sha2_intrinsics_arm.ipp"
