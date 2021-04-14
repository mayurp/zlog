//
//  reflection.cpp
//  template_fun
//
//  Created by Mayur Patel on 12/04/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#include "reflection.hpp"

namespace reflection
{
 
std::vector<Type>& getTypeRegistry()
{
    static std::vector<Type> registry;
    return registry;
}

}
