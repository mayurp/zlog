//
//  ctstring.hpp
//  logger
//
//  Created by Mayur Patel on 10/09/2021.
//

#ifndef ctstring_hpp
#define ctstring_hpp

// statically allocated string - not null terminated
template <size_t N>
struct FixedString
{
    constexpr std::string_view view() const { return { data, size }; }
    char data[N];
    size_t size = N; // default to entire buffer
};
                     
template <std::string_view const&... Strs>
struct join
{
    // Join all strings into a single std::array of chars
    static constexpr auto impl() noexcept
    {
        constexpr std::size_t len = (Strs.size() + ... + 0);
        std::array<char, len + 1> arr{};
        int i = 0;
        for (auto& s : {Strs...})
        {
            for (auto c : s) arr[i++] = c;
        }
        arr[len] = '\0';

        return arr;
    }
    // Give the joined string static storage
    static constexpr auto arr = impl();
    // View as a std::string_view
    static constexpr std::string_view value {arr.data(), arr.size() - 1};
};

// Helper to get the value out
template <std::string_view const&... Strs>
static constexpr auto join_v = join<Strs...>::value;

#endif /* ctstring_hpp */
