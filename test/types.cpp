#include <catch.hpp>

#include <tuple>

#include <alice/alice.hpp>

template<typename... S>
struct type_list
{
};

template<typename T, typename... S>
struct type_list<T, alice::cli<S...>>
{
    using type = alice::cli<T, S...>;
};

template<>
struct type_list<>
{
    using type = alice::cli<>;
};

struct nil{};

template<typename T, typename U>
struct cons{};

template<typename List>
struct list_to_cli;

template<>
struct list_to_cli<nil>
{
    using type = alice::cli<>;
};

template<typename Head, typename Tail>
struct list_to_cli<cons<Head, Tail>>
{
    using type = typename type_list<Head, typename list_to_cli<Tail>::type>::type;
};

TEST_CASE( "Cons list", "[meta]" )
{
    using list = cons<int, cons<char, cons<short, nil>>>;

    CHECK( std::is_same<list_to_cli<list>::type, alice::cli<int, char, short>>::value );
}

TEST_CASE( "Type concatenation", "[meta]" )
{
    CHECK( std::is_same<alice::cli<>, type_list<>::type>::value );

    using t_a = type_list<>::type;
    using t_b = type_list<int, t_a>::type;
    using t_c = type_list<char, t_b>::type;

    CHECK( std::is_same<alice::cli<char, int>, t_c>::value );
}
