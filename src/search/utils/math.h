#ifndef UTILS_MATH_H
#define UTILS_MATH_H

#include <limits>

namespace utils
{
/* Test if the product of two numbers is bounded by a third number.
   Safe against overflow. The caller must guarantee
   0 <= factor1, factor2 <= limit; failing this is an error. */

constexpr const float F_INFINITY = std::numeric_limits<float>::max();
constexpr const float F_NEG_INFINITY = std::numeric_limits<float>::min();


extern bool is_product_within_limit(int factor1, int factor2, int limit);
extern bool is_sum_within_limits(const float &x, const float &y);
extern bool is_less_than(const float &x, const float &y, const float &eps);
extern bool is_larger_than(const float &x, const float &y, const float &eps);
extern bool is_equals_to(const float &x, const float &y, const float &eps);
}

#endif
