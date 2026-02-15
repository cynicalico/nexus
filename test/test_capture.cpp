#include "nexus/nexus.hpp"

struct Add {
#if !defined(NEXUS_USE_STD_TYPE_INDEX)
    static constexpr nexus::Tag NEXUS_TAG{0xdeadbeef};
#endif
    int n;
};

struct Mul {
#if !defined(NEXUS_USE_STD_TYPE_INDEX)
    static constexpr nexus::Tag NEXUS_TAG{0xdeadbeef};
#endif
    int n;
};

int main(int, char *[]) {
    int i = 0;
    int expected = 0;

    auto nexus = nexus::Nexus();

    const auto id_1 = nexus.acquire_id();
    nexus.subscribe<Add>(id_1, [&i](const auto *p) { i += p->n + 1; });
    nexus.subscribe<Mul>(id_1, [&i](const auto *p) { i *= p->n + 1; });

    const auto id_2 = nexus.acquire_id();
    nexus.subscribe<Add>(id_2, [&i](const auto *p) { i += p->n + 2; });
    nexus.subscribe<Mul>(id_2, [&i](const auto *p) { i *= p->n + 2; });

    // basic operation
    nexus.publish<Add>(3);
    expected += (3 + 1) + (3 + 2);
    nexus.publish<Mul>(5);
    expected *= (5 + 1) * (5 + 2);

    // id_1 captures Add, Mul should still go to both
    nexus.capture<Add>(id_1);
    nexus.publish<Add>(7);
    expected += (7 + 1);
    nexus.publish<Mul>(11);
    expected *= (11 + 1) * (11 + 2);

    // test uncapture works
    nexus.uncapture<Add>(id_1);
    nexus.publish<Add>(13);
    expected += (13 + 1) + (13 + 2);

    // capturing two different types simultaneously is allowed
    nexus.capture<Mul>(id_1);
    nexus.publish<Mul>(15);
    expected *= (15 + 1);
    nexus.capture<Add>(id_2);
    nexus.publish<Add>(17);
    expected += (17 + 2);

    // unsubscribing should clear capture
    nexus.unsubscribe<Mul>(id_2);
    nexus.unsubscribe<Add>(id_2);
    nexus.publish<Add>(19);
    expected += (19 + 1);

    nexus.release_id(id_2);

    nexus.unsubscribe<Mul>(id_1);
    nexus.unsubscribe<Add>(id_1);
    nexus.release_id(id_1);

    if (i == expected) return 0;
    return 1;
}
