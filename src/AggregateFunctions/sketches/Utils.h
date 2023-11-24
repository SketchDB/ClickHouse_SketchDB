#pragma once

#include <base/types.h>

namespace DB
{

template <typename T>
Float64 median(std::vector<T> values)
{
    const size_t & n = values.size();
    std::sort(values.begin(), values.end());
    if (n % 2 == 0)
    {
		return static_cast<Float64>(values[(n-2) / 2] + values[n / 2]) / 2.0;
    }
    else
    {
		return static_cast<Float64>(values[(n-1) / 2]);
    }
}

}
