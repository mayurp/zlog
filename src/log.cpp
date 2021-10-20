//
//  log.cpp
//
//
//  Created by Mayur Patel on 12/04/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#include "log.hpp"

#include "magic_enum.hpp"

#include <sstream>
#include <map>
#include <regex>
#include <vector>


namespace logging
{

std::vector<LogMetaData>& get_registry()
{
    static std::vector<LogMetaData> registry;
    return registry;
}

void add_to_registry(const LogMetaData& data)
{
    get_registry().emplace_back(data);
}

}
