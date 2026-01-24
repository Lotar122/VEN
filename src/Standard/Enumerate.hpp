#pragma once
#include <cstddef>
#include <utility>
#include <iterator>

/*
    Wrote this. tried to write like the std for fun.
    *I wrote this because MSVC lags behind the standard
    ?This is a header which will dissapear when std::ranges::views::enumerate reaches full adoption

    It was such a cool thing i could pass on it so here you go. My own!
*/

namespace nihil
{
    template<typename Iter>
    class enumerate_iterator
    {
    public:
        using value_type = std::pair<std::size_t, decltype(*std::declval<Iter>())>;
        using reference = value_type;
        using pointer = void;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;

        enumerate_iterator(std::size_t index, Iter it)
            : index_(index), it_(it) {}

        reference operator*() const
        {
            return {index_, *it_};
        }

        enumerate_iterator& operator++()
        {
            ++index_;
            ++it_;
            return *this;
        }

        bool operator!=(const enumerate_iterator& other) const
        {
            return it_ != other.it_;
        }

    private:
        std::size_t index_;
        Iter it_;
    };

    template<typename Container>
    class enumerate_view
    {
    public:
        explicit enumerate_view(Container& c) : c_(c) {}

        auto begin() { return enumerate_iterator(std::size_t{0}, std::begin(c_)); }
        auto end() { return enumerate_iterator(std::size_t{0}, std::end(c_)); }

    private:
        Container& c_;
    };

    template<typename Container>
    enumerate_view<Container> enumerate(Container& c)
    {
        return enumerate_view<Container>(c);
    }
}
