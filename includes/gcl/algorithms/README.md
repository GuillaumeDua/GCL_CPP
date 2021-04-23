# **GCL C++** : Algorithms

This component provide - *constexpr* - algorithms used in the `gcl` library implementation.

## Details

| | |
| - | - |
| namespace | `gcl::algorithms` |
| path | `includes/gcl/algorithms` |

### **`gcl::algorithms::maths`**

| File | Element name | Description | example |
| ---- | ------------ | ----------- | ------- |
| `gcl/algorithms/maths.hpp` | maths::distance(T, T) | return the distance between two values | `distance(1, -1) == 2` |
| `gcl/algorithms/maths.hpp` | maths::abs(T) | absolute value with boundary safety<br> | `abs(-1) == 1` |

### **`gcl::algorithms::ranges`**

| File | Element name | Description | example |
| ---- | ------------ | ----------- | ------- |
| `gcl/algorithms/ranges.hpp` | ranges::is_in_range(Range, input) | return true if `intput` is in `range` | `is_in_range({1,2,3}, 2) == true` |
| `gcl/algorithms/ranges.hpp` | ranges::is_in_range(RangeIt, RangeIt, InputIt) | same as above, but using `begin`, `end` iterators | |
