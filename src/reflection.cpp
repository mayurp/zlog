//
//  reflection.cpp
//
//
//  Created by Mayur Patel on 12/04/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#include "reflection.hpp"

namespace reflection
{
 
std::vector<RegisteredType>& get_type_registry()
{
    static std::vector<RegisteredType> registry;
    return registry;
}

}
