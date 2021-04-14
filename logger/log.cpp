//
//  log.cpp
//  template_fun
//
//  Created by Mayur Patel on 12/04/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#include "log.hpp"

#include <vector>

namespace logging
{

std::vector<LogMetaData>& getRegistry()
{
    static std::vector<LogMetaData> registry;
    return registry;
}

void addToRegistry(const LogMetaData& data)
{
    getRegistry().emplace_back(data);
}

void addToVector(std::vector<std::string_view>& vec, const std::string_view& str)
{
    vec.emplace_back(str);
}

}
