//
//
//  Created by Mayur Patel on 27/03/2021.
//

#include "type_traits.hpp"

#include <list>
#include <vector>


static void test_type_traits()
{
    std::array sarr = {1, 2, 3};
    static_assert(is_iterable_v<decltype(sarr)>);

    int arr[3] = {1, 2, 3};
    static_assert(is_iterable_v<decltype((arr))>);

    std::vector<int> vec = {1, 2, 3};
    static_assert(is_iterable_v<decltype(vec)>);

    std::vector<bool> vecBools = {true, true, true};
    static_assert(is_iterable_v<decltype(vecBools)>);

    std::list<int> list = {1, 2, 3};
    static_assert(is_iterable_v<decltype(list)>);
}
