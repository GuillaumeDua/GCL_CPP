# **GCL C++** : Algorithms

This component provide - *constexpr* - algorithms used in the `gcl` library implementation.

## Details

| | |
| - | - |
| namespace | `gcl::algorithms` |
| path | `includes/gcl/algorithms` |

Grouped in : `gcl/algorithms/algorithms.hpp`

### **`gcl::algorithms::maths`**

Grouped in : `gcl/algorithms/algorithms/maths.hpp`

| Element name | Description | example |
| ------------ | ----------- | ------- |
| maths::distance(T, T) | return the distance between two values | `distance(1, -1) == 2` |
| maths::abs(T) | absolute value with boundary safety<br> | `abs(-1) == 1` |

### **`gcl::algorithms::ranges`**

Grouped in : `gcl/algorithms/algorithms/ranges.hpp`

| Element name | Description | example |
| ------------ | ----------- | ------- |
| ranges::is_in_range(Range, input) | return true if `intput` is in `range` | `is_in_range({1,2,3}, 2) == true` |
| ranges::is_in_range(RangeIt, RangeIt, InputIt) | same as above, but using `begin`, `end` iterators | |
