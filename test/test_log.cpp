//
//
//  Created by Mayur Patel on 27/03/2021.
//

#include "log.hpp"

#include <catch2/catch.hpp>


struct NonCopyable
{
    NonCopyable(int _i) : i(_i) {}
    NonCopyable(const NonCopyable& x) = delete;
    NonCopyable(NonCopyable&& x) = delete;
    NonCopyable& operator=(const NonCopyable& x);
    NonCopyable& operator=(NonCopyable&& x );
    
    int i = 0;
};
REFLECT(NonCopyable, i)

struct Aggregate
{
    Aggregate(int i) : n(i) {}
    NonCopyable n;
};
REFLECT(Aggregate, n)


// TODO: do more than just compile time checks
TEST_CASE("logging nocopy", "[log]")
{
    NonCopyable n(1);
    LOGD("test no copy lvalue: {value}", n);
    LOGD("test no copy rvalue: {value}", NonCopyable(1));
    
    Aggregate a(2);
    LOGD("test aggregate no copy lvalue: {value}", a);
    LOGD("test aggregate no copy rvalue: {value}", Aggregate(2));
}
