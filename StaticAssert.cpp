#include "Parser.Tool.hpp"
#include "Parser.hpp"

namespace parr
{
    template<std::size_t _need, typename T, std::size_t _sizeof = sizeof( T )>
    void static_assert_size_type()
    {
        static_assert( _need == _sizeof );
    }

    template<typename T1, typename T2, std::size_t sizeofT1 = sizeof( T1 ), std::size_t sizeofT2 = sizeof( T2 )>
    void static_assert_samesize_types()
    {
        static_assert( sizeofT1 == sizeofT2 );
    }

    template<typename T, std::size_t _sizeof = sizeof( T ), std::size_t _alignof = alignof( T )>
    void static_assert_samesizealign_type()
    {
        static_assert( _sizeof == _alignof );
    }

    template<typename T1, typename T2, std::size_t sizeofT1 = sizeof( T1 ), std::size_t sizeofT2 = sizeof( T2 ), std::size_t alignofT1 = alignof( T1 ), std::size_t alignofT2 = alignof( T2 )>
    void static_assert_samesizealign_types()
    {
        static_assert( sizeofT1 == sizeofT2 );
        static_assert( alignofT1 == alignofT2 );
    }

    [[maybe_unused]] static void StaticAsserts()
    {
        static_assert_samesizealign_types<Parser, Parser::Tool>();
    }
}  // namespace parr
