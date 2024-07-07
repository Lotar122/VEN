#pragma once
namespace nihil {
    namespace utils {
        template <typename To, typename From>
        To implicit_cast(const From& from) {
            return static_cast<To>(from);
        }
    }
};