#include "Parser.Tool.hpp"
#include "Parser.hpp"

namespace parr
{
    template<size_t need_, typename T, size_t sizeof_ = sizeof( T )>
    void static_assert_size_type()
    {
        static_assert( need_ == sizeof_ );
    }

    template<typename T1, typename T2, size_t sizeofT1 = sizeof( T1 ), size_t sizeofT2 = sizeof( T2 )>
    void static_assert_samesize_types()
    {
        static_assert( sizeofT1 == sizeofT2 );
    }

    template<typename T, size_t sizeof_ = sizeof( T ), size_t alignof_ = alignof( T )>
    void static_assert_samesizealign_type()
    {
        static_assert( sizeof_ == alignof_ );
    }

    template<typename T1, typename T2, size_t sizeofT1 = sizeof( T1 ), size_t sizeofT2 = sizeof( T2 ), size_t alignofT1 = alignof( T1 ), size_t alignofT2 = alignof( T2 )>
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
