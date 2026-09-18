#include <memory>
#include "nexus/nexus.hpp"

struct Add {
#if !defined(NEXUS_USE_STD_TYPE_INDEX)
    static constexpr nexus::Tag NEXUS_TAG{0xdeadbeef};
#endif
    int n;
};

int main(int, char *[]) {
    int i = 0;

    auto nexus = nexus::Nexus();

    const auto id = nexus.acquire_id();
    nexus.subscribe<Add>(id, [&i](const auto *p) { i += p->n; });

    const auto foo = Add{3};
    const auto bar = Add{5};
    const auto baz = std::make_unique<Add>(7);
    const auto qux = std::make_unique<Add>(9);
    nexus.publish_ptr<Add>(&foo);
    nexus.publish_ptr<Add>(&bar);
    nexus.publish_ptr<Add>(baz.get());
    nexus.publish_ptr<Add>(qux.get());

    nexus.unsubscribe<Add>(id);
    nexus.release_id(id);

    if (i == 3 + 5 + 7 + 9) return 0;
    return 1;
}
