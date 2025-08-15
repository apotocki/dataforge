/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <stdexcept>

namespace dataforge {

struct throw_error_handler
{
    static constexpr bool is_stateless = true;

    template <typename ... ArgTs>
    void operator()(const char* errmsg, ArgTs&& ...) const
    {
        throw std::runtime_error(errmsg);
    }
};

}
