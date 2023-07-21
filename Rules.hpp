#pragma once

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/uint8.hpp>

// clang-format-sh on

namespace parr::rule
{
    namespace pegtl = TAO_PEGTL_NAMESPACE;

    // aliases

    using any                         = pegtl::any;

    namespace meta
    {
        // catch between X and Y
        // match X, ignore Y
        template<typename X, typename Y>
        struct XtoYexcludeY           : pegtl::until<pegtl::at<Y>, X>{};

        template<typename... Rules>
        struct eat                    : pegtl::seq<pegtl::at<Rules...>, Rules...>{};
    }  // namespace meta

    //

    struct charParenOpen              : pegtl::one<'('>{};
    struct charParenClose             : pegtl::one<')'>{};

    namespace blanks
    {
        struct newlineDos             : pegtl::string<'\r', '\n'>{};
        struct newlineUnix            : pegtl::one<'\n'>{};

        struct eof                    : pegtl::eof{};
        struct eol                    : pegtl::sor<newlineUnix, newlineDos>{};
        struct eofl                   : pegtl::sor<eof, eol>{};
        struct eolf                   : pegtl::sor<eol, eof>{};
        struct eols                   : pegtl::plus<eol>{};

        struct spaces                 : pegtl::plus<pegtl::one<' '>>{};
        struct tabs                   : pegtl::plus<pegtl::one<'\t'>>{};

        struct stringCommentShort     : pegtl::two<'/'>{};
        struct stringCommentLongStart : pegtl::string<'/', '*'>{};
        struct stringCommentLongEnd   : pegtl::string<'*', '/'>{};

        /// match anything between string "//" and EOL/EOF
        /// EOL/EOF is NOT included in match
        struct commentShort           : pegtl::seq<stringCommentShort, pegtl::until<pegtl::at<eolf>, any>>{};
        /// match anything between string "/* and "*/"
        /// NO match if EOL is found before
        struct commentMedium          : pegtl::seq<stringCommentLongStart, pegtl::until<pegtl::at<stringCommentLongEnd>, pegtl::seq<pegtl::not_at<eolf>, any>>, stringCommentLongEnd>{};
        /// match anything between string "/*" and "*/"
        /// MUST be checked AFTER <commentMedium>
        struct commentLong            : pegtl::seq<stringCommentLongStart, pegtl::until<stringCommentLongEnd, any>>{};
        struct comment                : pegtl::sor<commentShort, commentMedium, commentLong>{};

        struct check                  : pegtl::sor<spaces, tabs, eols, comment>{};

        struct required               : pegtl::plus<check>{};
        struct optional               : pegtl::star<check>{};
    }  // namespace blanks

    namespace bom
    {
        struct utf8                   : pegtl::uint8::string<0xEF, 0xBB, 0xBF>{};
        struct utf32bigEndian         : pegtl::uint8::string<0x00, 0x00, 0xFE, 0xFF>{};
        struct utf32littleEndian      : pegtl::uint8::string<0xFF, 0XFE, 0x00, 0x00>{};
        struct utf16bigEndian         : pegtl::uint8::string<0xFE, 0xFF>{};
        struct utf16littleEndian      : pegtl::uint8::string<0xFF, 0XFE>{};
        struct utf7                   : pegtl::uint8::string<0x2B, 0x2F, 0x76>{};
        struct utf1                   : pegtl::uint8::string<0xF7, 0x64, 0x4C>{};
        struct utfEbcdic              : pegtl::uint8::string<0xDD, 0x73, 0x66, 0x73>{};
        struct scsu                   : pegtl::uint8::string<0x0E, 0xFE, 0xFF>{};
        struct bocu1                  : pegtl::uint8::string<0xFB, 0xEE, 0x28>{};
        struct gb18030                : pegtl::uint8::string<0x84, 0x31, 0x95, 0x33>{};

        struct check                  : pegtl::sor<utf8, utf32bigEndian, utf32littleEndian, utf16bigEndian, utf16bigEndian, utf16littleEndian, utf7, utf1, utfEbcdic, scsu, bocu1, gb18030>{};

        struct optional               : pegtl::opt<check>{};
    }  // namespace bom
}  // namespace parr::rule
