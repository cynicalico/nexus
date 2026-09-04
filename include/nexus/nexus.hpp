#pragma once

#include <concepts>
#include <functional>
#include <optional>
#include <ranges>
#include <span>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace nexus {
using ID = std::size_t;
using Tag = std::size_t;

#if !defined(NEXUS_USE_STD_TYPE_INDEX)
template<typename T>
concept IsNexusCombatible = std::same_as<decltype(T::NEXUS_TAG), const Tag>;
#else
template<typename T>
concept IsNexusCombatible = std::true_type::value;
#endif

class Nexus {
    using Payload = std::span<const std::byte>;
    using Receiver = std::function<void(Payload)>;

public:
    ID acquire_id();

    void release_id(ID id);

    template<typename T, typename Func>
        requires IsNexusCombatible<T> and std::invocable<Func, const T *>
    void subscribe(ID id, Func &&f);

    template<typename T>
        requires IsNexusCombatible<T>
    void unsubscribe(ID id);

    template<typename T, typename... Args>
        requires IsNexusCombatible<T>
    void publish(Args &&...args);

    template<typename T>
        requires IsNexusCombatible<T>
    std::optional<ID> get_capture();

    template<typename T>
        requires IsNexusCombatible<T>
    void capture(ID id);

    template<typename T>
        requires IsNexusCombatible<T>
    void uncapture(ID id, bool force = false);

private:
    ID next_id_ = 0;
    std::vector<ID> recycled_ids_{};

    std::unordered_map<Tag, std::optional<ID>> captures_;
    std::unordered_map<Tag, std::vector<Receiver>> receivers_;

    template<typename T>
        requires IsNexusCombatible<T>
    Tag type_tag_();

    template<typename T, typename... Args>
    std::span<std::byte> make_payload_(Args &&...args);
};
} // namespace nexus

inline nexus::ID nexus::Nexus::acquire_id() {
    if (recycled_ids_.empty()) return next_id_++;

    const auto id = recycled_ids_.back();
    recycled_ids_.pop_back();
    return id;
}

inline void nexus::Nexus::release_id(const ID id) {
    for (auto &receivers: receivers_ | std::views::values)
        if (receivers.size() > id) receivers[id] = nullptr;

    for (auto &capture: captures_ | std::views::values)
        if (capture && *capture == id) capture.reset();

    recycled_ids_.push_back(id);
}

template<typename T, typename Func>
    requires nexus::IsNexusCombatible<T> and std::invocable<Func, const T *>
void nexus::Nexus::subscribe(ID id, Func &&f) {
    const auto tag = type_tag_<T>();
    auto &receivers = receivers_[tag];
    if (receivers.size() <= id) receivers.resize(id + 1);
    receivers[id] = [f = std::forward<Func>(f)](const Payload buffer) {
        f(reinterpret_cast<const T *>(buffer.data()));
    };
}

template<typename T>
    requires nexus::IsNexusCombatible<T>
void nexus::Nexus::unsubscribe(ID id) {
    const auto tag = type_tag_<T>();
    auto &receivers = receivers_[tag];
    if (receivers.size() > id) {
        receivers[id] = nullptr;
        if (captures_[tag] && *captures_[tag] == id) captures_[tag].reset();
    }
}

template<typename T, typename... Args>
    requires nexus::IsNexusCombatible<T>
void nexus::Nexus::publish(Args &&...args) {
    const auto tag = type_tag_<T>();
    const auto payload = make_payload_<T>(std::forward<Args>(args)...);
    if (auto cap_id_opt = captures_[tag]; cap_id_opt) {
        if (receivers_[tag].size() > *cap_id_opt) {
            if (auto &r = receivers_[tag][*cap_id_opt]; r) r(payload);
        }
    } else {
        for (auto &r: receivers_[tag])
            if (r) r(payload);
    }
    operator delete(payload.data(), payload.size());
}

template<typename T>
    requires nexus::IsNexusCombatible<T>
std::optional<nexus::ID> nexus::Nexus::get_capture() {
    const auto tag = type_tag_<T>();
    if (captures_.contains(tag)) return captures_[tag];
    return std::nullopt;
}

template<typename T>
    requires nexus::IsNexusCombatible<T>
void nexus::Nexus::capture(ID id) {
    const auto tag = type_tag_<T>();
    captures_[tag] = id;
}

template<typename T>
    requires nexus::IsNexusCombatible<T>
void nexus::Nexus::uncapture(ID id, bool force) {
    const auto tag = type_tag_<T>();
    if (captures_[tag] && (*captures_[tag] == id || force)) captures_[tag].reset();
}

template<typename T>
    requires nexus::IsNexusCombatible<T>
nexus::Tag nexus::Nexus::type_tag_() {
#if !defined(NEXUS_USE_STD_TYPE_INDEX)
    return T::NEXUS_TAG;
#else
    return std::type_index(typeid(T)).hash_code();
#endif
}

template<typename T, typename... Args>
std::span<std::byte> nexus::Nexus::make_payload_(Args &&...args) {
    return std::span(reinterpret_cast<std::byte *>(new T{std::forward<Args>(args)...}), sizeof(T));
}
