# nexus

nexus is a simple [publish/subscribe](https://en.wikipedia.org/wiki/Publish%E2%80%93subscribe_pattern) implementation
that sends structs to and from endpoints. nexus requires either a tag data member on each struct you plan on sending
through it, or optionally a compile flag can be set to use `std::type_index`.

## Example

```cpp
#include "nexus/nexus.hpp"

struct Add {
    static constexpr nexus::Tag NEXUS_TAG{0xdeadbeef}; // not necessary with NEXUS_USE_STD_TYPE_INDEX enabled
    int n;
};

int main(int, char *[]) {
    auto nexus = nexus::Nexus();

    const auto id = nexus.acquire_id();
    nexus.subscribe<Add>(id, [&i](const auto *p) { i += p->n; });

    nexus.publish<Add>(3); // i now equals 3
    
    const auto id_2 = nexus.acquire_id();
    nexus.subscribe<Add>(id_2, [&i](const auto *p) { i += p->n; });
    
    nexus.publish<Add>(5); // i now equals 13

    return 0;
}
```

## Install

nexus is header-only, so you can just copy the `nexus` directory to your project and include `nexus.hpp`.

Alternatively, you can consume the file through CMake's FetchContent:

```cmake
set(NEXUS_USE_STD_TYPE_INDEX ON) # Optionally use std::type_index instead of a tag member
FetchContent_Declare(nexus GIT_REPOSITORY https://codeberg.org/cynicalico/nexus)
FetchContent_MakeAvailable(nexus)
```

Then link your project against `nexus::nexus`.

## Notes about `std::type_index`

`std::type_index` should be reliable in single translation units, but I'm not sure if it will be reliable across DLL
boundaries.

If this is a concern for you, you may not wish to enable this feature, and instead rely on explicitly
defined tags. A hash of the fully qualified name is a good alternative. This can be done easily with something
like [murmurst](https://codeberg.org/cynicalico/murmurst) (sorry for the shameless self-promotion).

## License

This project is licensed under the Unlicense.