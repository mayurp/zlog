#include <iostream>

#include "Common.hpp"

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

// TODO remove once in d3
#include <cassert>
#define CHECK_LOGIC(x) assert(x)
//

// Custom comparison and hashing functions which only consider specific indicies within the Node type
// Needed as we use std::sets to store a combined kay and value
template<typename Node, size_t I>
struct Compare
{
    // tells std::set comparator is capable of dealing with types other than key
    using is_transparent = std::true_type;

    // remove ref and const
    using KeyType = typename std::decay<decltype(std::declval<Node>(). template getKey<I>())>::type;

    bool operator() (const Node& lhs, const Node& rhs) const
    {
        return (lhs.template getKey<I>()) < (rhs.template getKey<I>());
    }
    
    bool operator()(const KeyType& lhs, const Node& rhs) const
    {
        return lhs < (rhs.template getKey<I>());
    }
    
    bool operator()(const Node& lhs, const KeyType& rhs) const
    {
        return (lhs.template getKey<I>()) < rhs;
    }
};

template<typename Node, size_t I>
struct Hash
{
    // remove ref and const
    using KeyType = typename std::decay<decltype(std::declval<Node>(). template getKey<I>())>::type;

    size_t operator() (const Node& n) const
    {
        return std::hash<KeyType>()(n.template getKey<I>());
    }
};

template<typename Node, size_t I>
struct KeyEqual
{
    bool operator()(const Node& lhs, const Node& rhs) const
    {
        return (lhs.template getKey<I>()) == (rhs.template getKey<I>());
    }
};

// Key specifications
template<bool isUnique, bool isOrdered, bool isMember>
struct BaseKey
{
    static constexpr bool ordered = isOrdered;
    static constexpr bool unique = isUnique;
    static constexpr bool memberBased = isMember;
    
    // Helper to get container types only at compile time
    template<typename Node, size_t I>
    static constexpr auto containerType()
    {
        if constexpr (isOrdered)
        {
            if constexpr (isUnique)
                return std::set<Node, Compare<Node, I>>();
            else
                return std::multiset<Node, Compare<Node, I>>();
        }
        else
        {
            if constexpr (isUnique)
                return std::unordered_set<Node, Hash<Node, I>, KeyEqual<Node, I>>();
            else
                return std::unordered_multiset<Node, Hash<Node, I>, KeyEqual<Node, I>>();
        }
    }
};

template<auto M, bool isUnique, bool isOrdered>
struct MemberKey : public BaseKey<isUnique, isOrdered, true>
{
    using KeyType = decltype(getPointerType(M));
    using NodeKeyType = uint8_t; // todo can we avoid this?
    static constexpr decltype(M) member = M;
};

// TODO make names less ugly??
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

template<typename T, bool isOrdered, bool isUnique>
struct NonMemberKey : public BaseKey<isUnique, isOrdered, false>
{
    using KeyType = T;
    using NodeKeyType = KeyType;
};

template<typename T>
struct UniqueOrderedNonMemberKey : public NonMemberKey<T, true, true>
{};

template<typename T>
struct NonUniqueOrderedNonMemberKey : public NonMemberKey<T, false, true>
{};

template<typename T>
struct NonUniqueUnorderedNonMemberKey : public NonMemberKey<T, false, false>
{};

template<typename T>
struct UniqueUnorderedNonMemberKey : public NonMemberKey<T, true, false>
{};

// Entry is the type of what is being stored
// KeySpecifier define which indices are used for lookups
// Supports values and pointers
template <typename Entry, typename ... KeySpecifier>
struct GeneralContainer
{
    // Stores Entry along with keys
    // Provides key lokup interfaces for custom comparison and hashing functions
    // Nodes manage a shared pointer to their underlying data
    // A shallow copy of a node is stored in each container (index)
    // This means:
    //   * Only 1 copy of the Entry its keys is stored
    //   * Copying the container is always a shallow copy and is fairly cheap
    //   * For this to be safe Nodes their data MUST be immutable.
    //     This matches the constness imposed by the underlying containers used std::set and set::unordered_set
    //   * Nodes are treated as values which simplifies how the are handled
    struct Node
    {
        Node(const Entry& _entry)
        {
            data = std::make_shared<Data>(_entry);
        }

        // TODO remove when we have lightweight node type
        Node()
        {
            data = std::make_shared<Data>();
        }

        const Entry& entry() const
        {
            CHECK_LOGIC(data);
            return data->entry;
        }

        auto dataUseCount() const
        {
            CHECK_LOGIC(data);
            return data.use_count();
        }
        
        // TODO remove when we have lightweight node type
        template<size_t I, typename Key>
        void setKey(const Key& key)
        {
            CHECK_LOGIC(data);
            using KeySpec = NthTypeOf<I, KeySpecifier...>;
            if constexpr (KeySpec::memberBased)
                return injectKey<I>(data->entry, key);
            else
            {
                std::get<I>(data->keys) = key;
            }

        }

        template<size_t I>
        const auto& getKey() const
        {
            CHECK_LOGIC(data);
            using KeySpec = NthTypeOf<I, KeySpecifier...>;
            if constexpr (KeySpec::memberBased)
                return extractKey<I>(data->entry);
            else
            {
                return std::get<I>(data->keys);
            }
        }
        
        // TODO do we need this?
        // Can we just compare data ptrs?
        bool operator== (const Node& rhs) const
        {
            CHECK_LOGIC(data);
            return data->entry == rhs.data->entry;
        }

    private:
        template<size_t I>
        static const auto& extractKey(const Entry& e)
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

        template<size_t I, typename Key>
        static void injectKey(Entry& e, const Key& key)
        {
            using KeySpec = NthTypeOf<I, KeySpecifier...>;
            constexpr auto& member = KeySpec::member;
            if constexpr (std::is_pointer_v<Entry>)
                e->*member = key;
            else if constexpr (is_shared_ptr<Entry>::value)
                (*e).*member = key;
            else
                e.*member = key;
        }
        
        // Actual storage
        struct Data
        {
            Data(const Entry& _entry) : entry(_entry)
            {}
            
            // TODO remove when we have lightweight node type
            Data()
            {}
            
            using Keys = std::tuple<typename KeySpecifier::NodeKeyType ...>;
            Keys keys;
            Entry entry;
        };
        
        std::shared_ptr<Data> data;
    };

    template<size_t I>
    const auto& get()
    {
        return std::get<I>(containers);
    }
    
    void insert(const Entry& e)
    {
        Node node(e); // todo check if node exists??
        insertLoop(node);
    }
    
    template<size_t I = 0, typename Key>
    void insert(const Key& key, const Entry& entry)
    {
        using KeySpec = NthTypeOf<I, KeySpecifier...>;
        static_assert(!KeySpec::memberBased, "Can't insert into member index with custom key");
        
        Node node(entry);
        node.template setKey<I>(key);
        insertLoop(node);
    }
    
    // Users should erase using key for efficiency
    void erase(const Entry& e)
    {
        Node node(e); // Don't know which key so need to make a node here
        eraseLoop(node);
    }
    
    template<size_t I = 0, typename Key>
    void erase(const Key& key)
    {
        const auto it = std::get<I>(containers).find(key);
        // TODO erase all if non unique container
        if (it != std::get<I>(containers).end())
        {
            std::cout << "Found node. Erasing from other indices" << std::endl;
            using KeySpec = NthTypeOf<I, KeySpecifier...>;
            const Node& node = *it; // TODO error checking
            eraseLoop(node);
        }
    }
    
    template<size_t I = 0, typename Key>
    auto find(const Key& key) const
    {
        using KeySpec = NthTypeOf<I, KeySpecifier...>;
        if constexpr (KeySpec::ordered)
            return std::get<I>(containers).find(key);
        else
        {
            // Need to create temporary Node for as unordered containers don't support
            // lookup by other than the stored type until c++20
            // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0919r1.html
            
            // TODO make lightweight node type for this
            Node node;
            node.template setKey<I>(key);
            return std::get<I>(containers).find(node);
        }
    }
    
    template<size_t I = 0, typename Key>
    auto count(const Key& key)
    {
        using KeySpec = NthTypeOf<I, KeySpecifier...>;
        if constexpr (KeySpec::ordered)
            return std::get<I>(containers).count(key);
        else
        {
            // Need to create temporary Node for as unordered containers don't support
            // lookup by other than the stored type until c++20
            // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0919r1.html
            
            // TODO make lightweight node type for this
            Node node;
            node.template setKey<I>(key);
            return std::get<I>(containers).count(node);
        }
    }

private:
    
    template<size_t I = 0>
    void insertLoop(const Node& n)
    {
        insertImpl<I>(n);
        if constexpr(I+1 != sizeof...(KeySpecifier))
            insertLoop<I+1>(n);
    }
    
    template<std::size_t I>
    void insertImpl(const Node& node)
    {
        std::get<I>(containers).insert(node);
    }
    
    template<size_t I = 0>
    void eraseLoop(const Node& n)
    {
        eraseImpl<I>(n);
        if constexpr(I+1 != sizeof...(KeySpecifier))
            eraseLoop<I+1>(n);
    }
    
    template<std::size_t I>
    void eraseImpl(const Node& n)
    {
        using KeySpec = NthTypeOf<I, KeySpecifier...>;
        std::get<I>(containers).erase(n);
        std::cout << "erase by node " << std::endl;
    }

    // Hacky way to pair KeySpecifiers with their Indices
    // Containers need to know which index they are for
    template<std::size_t... Is>
    static constexpr auto makeT(std::index_sequence<Is...>)
    {
        return std::tuple<decltype(KeySpecifier::template containerType<Node, Is>()) ...>();
    }

    decltype(makeT(std::make_index_sequence<sizeof...(KeySpecifier)>{})) containers;

    //std::tuple<decltype(KeySpecifier::template containerType<Node, 0>()) ...> containers;
};

