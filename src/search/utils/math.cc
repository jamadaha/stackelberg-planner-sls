#include "math.h"

#include <cassert>

namespace utils
{

bool is_product_within_limit(int factor1, int factor2, int limit)
{
    assert(factor1 >= 0);
    assert(factor2 >= 0);
    assert(limit >= 0);
    return factor2 == 0 || factor1 <= limit / factor2;
}


bool is_sum_within_limits(const float &, const float &)
{
// std::numeric_limits<float>::min() - x > y
    return true;
}

bool is_less_than(const float &x, const float &y, const float &eps)
{
    return x < y && x + eps < y;
}

bool is_larger_than(const float &x, const float &y, const float &eps)
{
    return x > y && x - eps > y;
}

bool is_equals_to(const float &x, const float &y, const float &eps)
{
    return x == y || (x > y && x - eps <= y) || (x < y && x + eps >= y);
}

}
