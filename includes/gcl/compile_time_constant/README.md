# **GCL C++** : Compile-time constant (ctc)

This component provide - *ctc* - algorithms to manipulate compile-time elements like `std::array` and `std::tuple`.  

## Details

| | |
| - | - |
| namespace | `gcl::ctc` |
| path | `includes/gcl/compile_time_constant` |

Grouped in : `gcl/compile_time_constant/ctc.hpp`

### **`gcl::ctc::algorithms`**

Grouped in `gcl/compile_time_constant/algorithms.hpp`

#### **`gcl::ctc::algorithms::array`**

Grouped in `gcl/compile_time_constant/algorithms/array.hpp`

| Element name | Description | example |
| ------------ | ----------- | ------- |
| `deduplicate<auto... values>()`   | return a `std::array<T,N>` that contains deduplicated values,<br>where `T` is `std::common_type_t<decltype(values)...>` and `N` the count of unique values | `deduplicate<1,2,3,1,2,3,2>() == std::array{1,2,3}` |
| `deduplicate(const Ts... values)` | return a `std::pair`, where :<br>- first is an `std::array<T, N>` with T is `std::common_type_t<Ts...>` and N is `sizeof...(Ts)`<br>- `second` is the position of the last unique element in `first` | see example below |
| `shrink<std::size_t position>(std::array<T,N>)` | return a new `std::array<T, N_p>`, where `N_p` equal to `N`, and elements up to `N` as content | see example below |

Example :

```cpp
#include <gcl/compile_time_constant/ctc.hpp>
// or gcl/compile_time_constant/algorithms.hpp
// or gcl/compile_time_constant/algorithms/array.hpp

namespace ctc_array_algorithms = gcl::ctc::algorithms::array;
constexpr auto values = []() consteval
{
    constexpr auto deduplicate_result =
        ctc_array_algorithms::deduplicate(1, 2.0f, 1.0f, 2, 3, 4, 4e0, 0x4, char{1}, 5, 1, 2, 3); // common type : double
    constexpr auto deduplicated_values =
        std::get<0>(deduplicate_result); // structure-binding not allowed in constant-expression
    constexpr auto last_unique = std::get<1>(deduplicate_result);
    return ctc_array_algorithms::shrink<last_unique>(deduplicated_values);
}
();
constexpr auto expected_result = std::array<double, 5>{1, 2, 3, 4, 5};
static_assert(std::is_same_v<decltype(values), decltype(expected_result)>);
static_assert(values == expected_result);
```

### **`gcl::ctc::algorithms::tuple`**

| Element name | Description | example |
| ------------ | ----------- | ------- |
| tuple::tuple_to_std_array(TupleType value) | convert `value` into a  | `is_in_range({1,2,3}, 2) == true` |

