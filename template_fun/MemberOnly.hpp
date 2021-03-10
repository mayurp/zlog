#include <iostream>

#include "Common.hpp"

#include <set>
#include <unordered_set>

// test key templates ------------------------

template<typename Entry, auto member>
struct Compare
{
    using KeyType = decltype(getPointerType(member));

    //static_assert(std::is_same_v<Entry, ValueType>, "Member Key doesn't belong to Entry type");
    
    // tells std::set comparator is capable of dealing with types other than key
    using is_transparent = std::true_type;
    
    bool operator() (const Entry& lhs, const Entry& rhs) const
    {
        return lhs.*member < rhs.*member;
    }
    
    bool operator()(const KeyType& lhs, const Entry& rhs) const
    {
        return lhs < rhs.*member;
    }
    
    bool operator()(const Entry& lhs, const KeyType& rhs) const
    {
        return lhs.*member < rhs;
    }
};

template<typename Entry, auto member>
struct Hash
{
    using KeyType = decltype(getPointerType(member));

    // tells std::set comparator is capable of dealing with types other than key
    using is_transparent = std::true_type;

    bool operator() (const Entry& e) const
    {
        return std::hash<KeyType>()(e.*member);
    }
    
    bool operator() (const KeyType& k) const
    {
        return std::hash<KeyType>()(k);
    }
};

template<typename KeyType>
struct KeyEqual
{
    using is_transparent = std::true_type;
    
    bool operator()(const KeyType & k1, const KeyType& k2) const
    {
        return k1 == k2;
    }
};

//template<typename T>
//struct Key
//{
//    using KeyType = T;
//    template<typename Entry>
//    static std::unordered_map<KeyType, Entry> containerType();
//};

//template<typename T, bool isOrdered, bool isUnique>
//struct NonMemberKey
//{
//    using KeyType = T;
//
//    template<typename Entry>
//    static constexpr auto containerType()
//    {
//        if constexpr (isOrdered)
//        {
//            if constexpr (isUnique)
//                return std::map<KeyType, Entry>();
//            else
//                return std::multimap<KeyType, Entry>();
//        }
//        else
//        {
//            if constexpr (isUnique)
//                return std::unordered_map<KeyType, Entry>();
//            else
//                return std::unordered_multimap<KeyType, Entry>();
//        }
//    }
//};
//
//template<typename T>
//struct UniqueOrderedNonMemberKey : public NonMemberKey<T, true, true>
//{};
//
//template<typename T>
//struct NonUniqueOrderedNonMemberKey : public NonMemberKey<T, false, true>
//{};
//
//template<typename T>
//struct NonUniqueUnorderedNonMemberKey : public NonMemberKey<T, false, false>
//{};
//
//template<typename T>
//struct UniqueUnorderedNonMemberKey : public NonMemberKey<T, true, false>
//{};


template<auto M, bool isUnique, bool isOrdered>
struct MemberKey
{
    using KeyType = decltype(getPointerType(M));
    using ValueType = decltype(getClassType(M));

    static constexpr decltype(M) member = M;
    
    template<typename Entry>
    static constexpr auto containerType()
    {
        if constexpr (isOrdered)
        {
            if constexpr (isUnique)
                return std::set<Entry, Compare<Entry, M>>();
            else
                return std::multiset<Entry, Compare<Entry, M>>();
        }
        else
        {
            if constexpr (isUnique)
                return std::unordered_set<Entry, Hash<Entry, M>, KeyEqual<KeyType>>();
            else
                return std::unordered_multiset<Entry, Hash<Entry, M>, KeyEqual<KeyType>>();
        }
    }
};

template<auto M>
struct UniqueOrderedMemberKey : public MemberKey<M, true, true>
{};

template<auto M>
struct NonUniqueOrderedMemberKey : public MemberKey<M, false, true>
{};

template<auto M>
struct NonUniqueUnorderedMemberKey : public MemberKey<M, false, false>
{};

template<auto M>
struct UniqueUnorderedMemberKey : public MemberKey<M, true, false>
{};

//template<auto M>
//struct UnorderedMemberKey
//{
//    using KeyType = decltype(getPointerType(M));
//    static constexpr decltype(M) member = M;
//
//
//    template<typename Entry>
//    static std::unordered_set<Entry, Hash<Entry>> containerType();
//};

template <typename Entry, typename ... KeySpecifier>
struct MemberOnlyContainer
{
    template<size_t I>
    const auto& get()
    {
        return std::get<I>(containers);
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
        //using KeyType = NthTypeOf<1, Key...>;
        //constexpr auto keyMember = KeyType::member;
        //std::get<1>(maps)[e.*keyMember] = e;
        //        std::get<0>(containers)["someString"] = e;
        //        std::get<1>(containers)["someOtherString"] = e;
        //        std::get<2>(containers).insert(e);

    
        const auto it = std::get<I>(containers).find(key);
        if (it != std::get<I>(containers).end())
        {
            const Entry& entry = *it;
            erase(entry);
        }
    }
    
    template<size_t I = 0, typename Key>
    auto find(const Key& key)
    {
        return std::get<I>(containers).find(key);
    }
    
    template<size_t I = 0, typename Key>
    auto count(const Key& key)
    {
        return std::get<I>(containers).count(key);
    }

private:

    template<size_t I>
    const auto& getKey(const Entry& e)
    {
        using KeySpec = NthTypeOf<I, KeySpecifier...>;
        constexpr auto& member = KeySpec::member;
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
        if constexpr(I+1 != sizeof...(KeySpecifier))
            insertLoop<I+1>(e);
    }
    
    template<std::size_t I>
    void insertImpl(const Entry& e)
    {
        //const auto& key = getKey<I>(e);
        std::get<I>(containers).insert(e);
        //std::cout << "insert by key " << key << std::endl;
    }
    
    template<size_t I = 0>
    void eraseLoop(const Entry& e)
    {
        eraseImpl<I>(e);
        if constexpr(I+1 != sizeof...(KeySpecifier))
            eraseLoop<I+1>(e);
    }
    
    template<std::size_t I>
    void eraseImpl(const Entry& e)
    {
        const auto& key = getKey<I>(e);
        std::get<I>(containers).erase(key);
        std::cout << "erase by key " << key << std::endl;
    }

    std::tuple<decltype(KeySpecifier::template containerType<Entry>()) ...> containers;
};
