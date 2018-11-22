# GCL_CPP
### Guss's Common library, in CPP

This library is a stack of useful and convinient components that makes my projects and everyday jobs way easier

Each **component** *(split by namespace)* aims to be :
- **easy-to-use**
- **easy-to-maintain**
- **powerful**.

### table-of-content :
| **component** name                 | description                                                                                |
|------------------------------------|--------------------------------------------------------------------------------------------|
| `mp`                               | meta-programming functions *(partial template definition, require/constraints, ...)*       |
| `tuple_utils`                      | functions to extend std::tuple functionalities.<br>*(type_at, index_of, for_each, ...)*    |
| `test`                             | stand-alone test library                                                                   |
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

### incomplete
| **component** name               | description                              |                   todo                          |
|----------------------------------|------------------------------------------|-------------------------------------------------|
| io                               | basic std::ostream/std::istream wrapper  |  merge with gcl\:\:poc\:\:fd_proxy              |
| maths                            | basic maths functions                    |  C++17 refactoring / constexpr                  |

### buggy
| **component** name               | description                              |                   issue                         |
|----------------------------------|------------------------------------------|-------------------------------------------------|
| color                            | color for console output                 | MSVC : no colors in Windows console             |
