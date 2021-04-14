#include <iostream>

#include "Common.hpp"

#include <map>

template <typename Entry, auto ... Member>
struct SimpleContainer
{
    template<size_t I>
    const auto& get()
    {
        return std::get<I>(maps);
    }
    
    void insert(const Entry& e)
    {
        insertLoop(e);
    }

    void erase(const Entry& e)
    {
        eraseLoop(e);
    }
    
    template<size_t I = 0, typename Key>
    void erase(const Key& key)
    {
        const Entry& entry = std::get<I>(maps)[key];
        erase(entry);
    }

    template<size_t I = 0, typename Key>
    auto find(const Key& key)
    {
        return std::get<I>(maps).find(key);
    }

    template<size_t I = 0, typename Key>
    auto count(const Key& key)
    {
        return std::get<I>(maps).count(key);
    }

private:

    template<size_t I>
    const auto& getKey(const Entry& e) const
    {
        constexpr auto tup = std::make_tuple(Member...);
        constexpr auto member = std::get<I>(tup);
        if constexpr (std::is_pointer_v<Entry>)
            return e->*member;
        else if constexpr (is_shared_ptr<Entry>::value)
            return (*e).*member;
        else
            return e.*member;
    }

    template<size_t I = 0>
    void insertLoop(const Entry& e)
    {
        insertImpl<I>(e);
        if constexpr(I+1 != sizeof...(Member))
            insertLoop<I+1>(e);
    }
    
    template<std::size_t I>
    void insertImpl(const Entry& e)
    {
        const auto& key = getKey<I>(e);
        std::get<I>(maps)[key] = e;
        std::cout << "insert by key " << key << std::endl;
    }

    template<size_t I = 0>
    void eraseLoop(const Entry& e)
    {
        eraseImpl<I>(e);
        if constexpr(I+1 != sizeof...(Member))
            eraseLoop<I+1>(e);
    }

    template<std::size_t I>
    void eraseImpl(const Entry& e)
    {
        const auto& key = getKey<I>(e);
        std::get<I>(maps).erase(key);
        std::cout << "erase by key " << key << std::endl;
    }

    std::tuple<std::map<decltype(getPointerType(Member)), Entry>...> maps;
};


