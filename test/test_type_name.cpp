#include "type_name.hpp"


// Compile time checks

struct Test1
{};

enum TestEnum1
{};

enum class TestEnumClass1
{};

namespace
{
struct Test2
{};

enum TestEnum2
{};

enum class TestEnumClass2
{};
}

namespace blah
{
struct Test3
{};

enum TestEnum3
{};

enum class TestEnumClass3
{};
}

static_assert(type_name_v<Test1> == "Test1");
static_assert(type_name_v<TestEnumClass1> == "TestEnumClass1");
static_assert(type_name_v<TestEnum1> == "TestEnum1");

static_assert(type_name_v<Test2> == "Test2");
static_assert(type_name_v<TestEnumClass2> == "TestEnumClass2");
static_assert(type_name_v<TestEnum2> == "TestEnum2");

static_assert(type_name_v<blah::Test3> == "Test3");
static_assert(type_name_v<blah::TestEnumClass3> == "TestEnumClass3");
static_assert(type_name_v<blah::TestEnum3> == "TestEnum3");


// TODO: Add reflection string type so this isn't needed for CTF meta data generation
static_assert(type_name_v<char*> == "string");
static_assert(type_name_v<const char*> == "string");
static_assert(type_name_v<std::string> == "string");
static_assert(type_name_v<std::string_view> == "string");

static_assert(type_name_v<int8_t> == "int8_t");
static_assert(type_name_v<int16_t> == "int16_t");
static_assert(type_name_v<int32_t> == "int32_t");
static_assert(type_name_v<int64_t> == "int64_t");

static_assert(type_name_v<uint8_t> == "uint8_t");
static_assert(type_name_v<uint16_t> == "uint16_t");
static_assert(type_name_v<uint32_t> == "uint32_t");
static_assert(type_name_v<uint64_t> == "uint64_t");


// TODO: Add reflection Integer type so this isn't needed for CTF meta data generation
static_assert(type_name_v<int> == "int32_t");
//static_assert(type_name_v<long> == "int64_t");
static_assert(type_name_v<long long> == "int64_t");
static_assert(type_name_v<unsigned int> == "uint32_t");
//static_assert(type_name_v<unsigned long> == "uint64_t");
static_assert(type_name_v<unsigned long long> == "uint64_t");
