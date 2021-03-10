//
//  log.hpp
//  template_fun
//
//  Created by Mayur Patel on 10/03/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#ifndef log_h
#define log_h

#include <type_traits>
#include <string_view>
#include <string>
#include <iostream>
#include <sstream>
#include <map>

namespace log
{

struct LogMacroData
{
    std::string_view format;
    std::string_view file;
    int32_t line;
};

inline std::map<int, LogMacroData>& getRegistry()
{
    static std::map<int, LogMacroData> registry;
    return registry;
}

template <typename MetaData>
struct MetaDataNode
{
    MetaDataNode() : id(log_id++)
    {
        getRegistry()[id] = macroData;
    }
    
    int32_t id;
    LogMacroData macroData = MetaData{}();
};

template<typename MetaData>
inline MetaDataNode<MetaData> meta_data_node{};


template <typename MetaData, typename... Args>
void logFunc(Args... args)
{
    static const auto m = meta_data_node<MetaData>;
    std::cout << m.id;
    //std::cout << m.format << " " << m.file << " " << m.line << ", ";
    //((std::cout << ctti::nameof<decltype(args)>() << " " << args  << std::endl), ...);
    std::cout << std::endl;
}

#define LOGID(format, ...)                                      \
do                                                              \
{                                                               \
    using namespace log;                                        \
    struct                                                      \
    {                                                           \
        constexpr LogMacroData operator()() const noexcept      \
        {                                                       \
            return LogMacroData{format, __FILE__, __LINE__};    \
        }                                                       \
    } anonymous_meta_data;                                      \
    logFunc<decltype(anonymous_meta_data)>(__VA_ARGS__);   \
} while(false)                                                  \

}
#endif /* log_h */
