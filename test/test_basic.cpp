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

    nexus.publish<Add>(3);
    nexus.publish<Add>(5);
    nexus.publish<Add>(7);
    nexus.publish<Add>(9);

    nexus.unsubscribe<Add>(id);
    nexus.release_id(id);

    if (i == 3 + 5 + 7 + 9) return 0;
    return 1;
}
