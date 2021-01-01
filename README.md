# GCL_CPP

This **modern-C++**, **header-only library** is a stack of useful and convinient components that make my everyday projects & jobs way easier

Each **component** *(split by namespace)* aims to be :
- **easy-to-use**
- **easy-to-maintain**
- **powerful**.

> **NB :**  
> This library is a never-ending WIP, as it matches needs according to the C++ standards and compilers implementations.  
> Thus, many components only exists to fill STL holes, and so are likely to disappear when standard features are implemented *-and released-* in the standard and by compilers.  
> *If at some point, a compiler does not support a specific feature, this information will be register as a limitation in-code comment*  
> - Example : Clang 11.0.0 does not implement `"Lambdas in unevaluated contexts" (P0315R4)`

## Versions

- **`v2`** is a WIP with C++17/2a implementations
- **`v1`** is a legacy tag that still exists for retro-compatibility  
  *Offers C++11/14/17 implementations in `gcl` namespace,  
  as well as C++98/03 implementations in `gcl::deprecated` namespace

## Features

### table-of-content :
| **component** name                 | description                                                                                |
|------------------------------------|--------------------------------------------------------------------------------------------|
| `mp`                               | meta-programming functions *(partial template definition, require/constraints, ...)*       |
| `tuple_utils`                      | functions to extend std::tuple functionalities.<br>*(type_at, index_of, for_each, ...)*    |
| `test`                             | stand-alone test library                                                                   |
| `pattern::ecs`                     | ECS (Entity Component System) design                                                       |
| `container::component_aggregator`  | generic variadic CRTP + type-erasure                                                       |
| `container::polymorphic_vector`    | contiguous container that store type-erased datas<br>allow per-type fast linear access<br>allow linear fast access as well     |
| `container::entity_vector`         | same as container::polymorphic_vector<br>add per-properties fast linear access             |
| `container::polymorphic_reference` | reference wrapper to type-erased value                                                     |
| `container::linear_vector`         | wrapper to a std::vector<std::unique_ptr\<T\>><br>elements have constant memory place      |
| `functionnal`                      | function traits,<br>bunch of functions to combine homogeneous/heterogenous functions       |
| `introspection`                    | SFINAE detectors for nested types, class members, and functions                            |
| `io`                               | basic std::ostream/std::istream wrapper                                                    |
| `serialization`                    | generic serializer with static/dynamic polymorphism<br>see my [talk at CppFRug](https://github.com/cpp-frug/paris/tree/master/events/2017-01-19_n14/Serial)              |
| `type_index`                       | static-to-dynamic and dynamic-to-static brige for class instanciation                      |
| `type_info`                        | helpers to type informations<br>*(type_id, variadic_template::index_of, variadic_template::type_at, ... )*<br>and *constexpr std::string_view experimental::type_name\<T\>* |
| `color`                            | colors for console output                                                                  |

### incomplete
| **component** name               | description                              |                   todo                          |
|----------------------------------|------------------------------------------|-------------------------------------------------|
| `io`                             | basic std::ostream/std::istream wrapper  |  merge with gcl\:\:poc\:\:fd_proxy              |
| `maths`                          | basic maths functions                    |  C++17 refactoring / constexpr                  |

### other namespaces :
| **component** name                 | description                                                                                |
|------------------------------------|--------------------------------------------------------------------------------------------|
| `gcl::<component_name>::experimental` | features that are still in development and may or may not work properly.<br>*(not fully tested, may rely on a language bug/hack, ...)* |
| `gcl::test`                        | use for both `gcl::test` component, and all tests<br>*(e.g `gcl::test::<component_name>`)* |
| `gcl::deprecated`                  | deprecated components implementation<br>*(c++0x, c++11)*<br>interface may change between a component like `gcl::deprecated::<component_name>` and `gcl::<component_name>` |
| `gcl::old`                         | shameful C++98/0x components that are not supported anymore                                 |

### About the name :
> `GCL` stands for `Guss's Common Library`
