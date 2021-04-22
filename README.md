# **GCL C++**

[![CMake_Clang_latest](https://github.com/GuillaumeDua/GCL_CPP/actions/workflows/cmake_clang_latest.yml/badge.svg?branch=master&event=push)](https://github.com/GuillaumeDua/GCL_CPP/actions/workflows/cmake_clang_latest.yml)
[![CMake_GCC_10](https://github.com/GuillaumeDua/GCL_CPP/actions/workflows/cmake_gcc_10.yml/badge.svg?branch=master&event=push)](https://github.com/GuillaumeDua/GCL_CPP/actions/workflows/cmake_gcc_10.yml)
[![MSBuild](https://github.com/GuillaumeDua/GCL_CPP/actions/workflows/msbuild.yml/badge.svg?branch=master&event=push)](https://github.com/GuillaumeDua/GCL_CPP/actions/workflows/msbuild.yml)

:construction: **WIP** : [see milestone v1 -> v2](https://github.com/GuillaumeDua/GCL_CPP/milestone/2)

---

This **modern-C++**, **header-only library** is a stack of useful and convinient components that make my everyday projects & jobs way easier.

Each **component** *(spli by namespaces)* aims to be :

- **easy-to-use**
- **easy-to-maintain**
- **powerful**

> **NB :** This library is a never-ending WIP, as it matches needs according to the C++ standards and compilers implementations.  
> Thus, many components only exists to fill what I consider to be STL holes, and so are likely to disappear when standard features are implemented *-and released-* in the standard and by compilers.  

## Build

This library is header-only, meaning you only need to add `includes/gcl` to your include path.

However, a `CMake` target exposes an `gcl_cpp` INTERFACE library target that you can integrate into your build.  

Currently, the only available option is `gcl_cpp_BUILD_TEST` - which is set to `OFF` by default - that generates a binary to run runtime tests.

## Tests

As this library components are mainly template-metaprogramming or constexpr ones, most of the tests are processed at **`compile-time`**.  

Currently, there is no option to disable compile-time tests.  
*If you need such option, create a Github issue, or make a pull-request.*

As mentioned in the previous section, a CMake target can be generate, when enabling the `gcl_cpp_BUILD_TEST` option.  
However, it only cover runtime tests.

## Versions

| Name | Description |
| ---- | ----------- |
| **`v2`** | [WIP/refactoring](https://github.com/GuillaumeDua/GCL_CPP/milestone/2) to only use C++17/2a/20 implementations |
| **`v1`** | **Legacy** tag that still exists for projects that depends on, but is no longer maintained<br>Offers C++11/14/17 implementations in `gcl` namespace<br>as well as C++98/03 implementations in `gcl::deprecated` namespace |

## Features

### table-of-content

| **component** name | description                                                                                |
|--------------------|--------------------------------------------------------------------------------------------|
| `mp`           | meta-programming elements to provide computation at compile-time       |
| `cx`           | constexpr elements |
| `ctc`          | compile-time constants.<br>mainly provides algorithms to manipulate `std::array` and `std::tuple` at compile-time |
| `io`           | io manipulation, mainly for serialization |
| `container`    | containers |
| `pattern`      | mid-level design patterns, such as ECS |
| [algorithms](./includes/gcl/algorithms/README.md)   | some algorithms |
| `functional`   | function-related elements |
| `concepts`     | concepts definition. Note that most concepts are defined within components they are related to.<br>For instance, `gcl::mp::concepts` and `gcl::io::concepts` |


## Compilers support

This library aims to compile using the following compilers :

- `GCC`
- `Clang`
- `MsVC-CL`
- `MS Clang-CL`

*If at some point, a compiler does not support a specific feature, this information will be register as a limitation in-code comment  
Also, a warning will be generated at compile-time accordingly.*

> - Example : `Clang 11.0.0` does not implement `"Lambdas in unevaluated contexts" (P0315R4)`

### Currently known limitations

#### **`Clang`** / **`Clang-CL`**

| File | Element | Description |
| --------- | ------- | ----------- |
| gcl/mp/pack_traits.hpp | `gcl::mp::type_traits::index_of<T, Ts...>` | uses an alternative implementation that use recursion, in opposition to other compilers |
| gcl/mp/pack_traits.hpp | `gcl::mp::pack_traits<...>::index_of_v`<br>`gcl::mp::pack_traits<...>::first_index_of_v`<br>`gcl::mp::pack_traits<...>::last_index_of_v` | Known limitation of Clang 12.0.0<br>*Invalid operands to binary expression ('const auto' and 'int')* |

#### **`GCC`**

None.

#### **`MsVC-CL`**

None.

### About the name

> `GCL` stands for `Guss's Common Library`
