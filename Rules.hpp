#pragma once

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/uint8.hpp>

// clang-format-sh on

namespace parr::rule
{
    namespace pegtl = TAO_PEGTL_NAMESPACE;

    // aliases

    using any                          = pegtl::any;

    namespace meta
    {
        // catch between X and Y
        // match X, ignore Y
        template<typename X, typename Y>
        struct XtoYexcludeY            : pegtl::until<pegtl::at<Y>, X>{};

        template<typename Rule>
        struct peek                    : pegtl::seq<pegtl::at<Rule>, Rule>{};

        template<typename... Rules>
        struct optOr                   : pegtl::opt<pegtl::sor<Rules...>>{};
    }  // namespace meta

    //

    using charParenOpen                = pegtl::one<'('>;
    using charParenClose               = pegtl::one<')'>;

    // newlines

    using charNewlineN                 = pegtl::one<'\n'>;
    using charNewlineR                 = pegtl::one<'\r'>;

    struct bol                         : pegtl::bol{};

    struct eof                         : pegtl::eof{};
    struct eolDos                      : pegtl::seq<charNewlineR, charNewlineN>{};
    struct eolUnix                     : charNewlineN{};
    struct eol                         : pegtl::sor<eolUnix, eolDos>{};

    struct eofl                        : pegtl::sor<eof, eol>{};
    struct eolf                        : pegtl::sor<eol, eof>{};

    // comments
    //
    //  comentShort    match anything between string "//" and EOL/EOF
    //                 EOL/EOF is NOT included in match
    //  commentMedium  match anything between string "/* and "*/"
    //                 NO match if EOL is found before
    //  commentLong    match anything between string "/*" and "*/"
    //                 MUST be checked AFTER [commentMedium]

    using stringCommentShort           = pegtl::two<'/'>;
    using stringCommentLongStart       = pegtl::ascii::string<'/', '*'>;
    using stringCommentLongEnd         = pegtl::ascii::string<'*', '/'>;

    struct commentShort                : pegtl::seq<stringCommentShort, pegtl::until<pegtl::at<eolf>, any>>{};  //meta::XtoYexcludeY<any, eolf>>{};
    struct commentMedium               : pegtl::seq<stringCommentLongStart, pegtl::until<pegtl::at<stringCommentLongEnd>, pegtl::seq<pegtl::not_at<eolf>, any>>, stringCommentLongEnd>{};
    struct commentLong                 : pegtl::seq<stringCommentLongStart, pegtl::until<stringCommentLongEnd, any>>{};
    struct comment                     : pegtl::sor<pegtl::sor<commentShort, commentMedium, commentLong>>{};
    struct commentOpt                  : pegtl::opt<comment>{};

    // blanks

    using spaceOrTab                   = pegtl::one<' ', '\t'>;
    using spacesOrTabs                 = pegtl::plus<spaceOrTab>;
    using spacesOrTabsOpt              = pegtl::opt<spacesOrTabs>;

    struct blankWithCommentShort       : pegtl::plus<pegtl::sor<spacesOrTabs, commentShort, commentMedium>>{};
    struct blankWithCommentShortOpt    : pegtl::opt<blankWithCommentShort>{};
    struct blankWithCommentLong        : pegtl::plus<pegtl::sor<spacesOrTabs, eol, commentShort, commentMedium, commentLong>>{};
    struct blankWithCommentLongOpt     : pegtl::star<pegtl::sor<spacesOrTabs, eol, commentShort, commentMedium, commentLong>>{};
    struct blankWithComment            : pegtl::sor<blankWithCommentLong, blankWithCommentShort>{};
    struct blankWithCommentOpt         : pegtl::opt<blankWithComment>{};

    struct blankWithoutCommentShort    : spacesOrTabs{};
    struct blankWithoutCommentShortOpt : pegtl::opt<blankWithoutCommentShort>{};
    struct blankWithoutCommentLong     : pegtl::plus<pegtl::sor<spacesOrTabs, eolf>>{};
    struct blankWithoutCommentLongOpt  : pegtl::opt<blankWithoutCommentLong>{};
    struct blankWithoutComment         : pegtl::sor<blankWithoutCommentShort>{};
    struct blankWithoutCommentOpt      : pegtl::opt<blankWithoutComment>{};

    struct bom
    {
        struct utf8                    : pegtl::uint8::string<0xEF, 0xBB, 0xBF>{};
        struct utf32bigEndian          : pegtl::uint8::string<0x00, 0x00, 0xFE, 0xFF>{};
        struct utf32littleEndian       : pegtl::uint8::string<0xFF, 0XFE, 0x00, 0x00>{};
        struct utf16bigEndian          : pegtl::uint8::string<0xFE, 0xFF>{};
        struct utf16littleEndian       : pegtl::uint8::string<0xFF, 0XFE>{};
        struct utf7                    : pegtl::uint8::string<0x2B, 0x2F, 0x76>{};
        struct utf1                    : pegtl::uint8::string<0xF7, 0x64, 0x4C>{};
        struct utfEbcdic               : pegtl::uint8::string<0xDD, 0x73, 0x66, 0x73>{};
        struct scsu                    : pegtl::uint8::string<0x0E, 0xFE, 0xFF>{};
        struct bocu1                   : pegtl::uint8::string<0xFB, 0xEE, 0x28>{};
        struct gb18030                 : pegtl::uint8::string<0x84, 0x31, 0x95, 0x33>{};

        struct check                   : pegtl::sor<utf8, utf32bigEndian, utf32littleEndian, utf16bigEndian, utf16bigEndian, utf16littleEndian, utf7, utf1, utfEbcdic, scsu, bocu1, gb18030>{};
    };

    // lines

    struct lineBlankWithComment        : pegtl::seq<bol, blankWithComment, eol>{};
    struct lineBlankWithoutComment     : pegtl::seq<bol, blankWithoutComment, eol>{};
    struct lineBlank                   : pegtl::sor<lineBlankWithoutComment, lineBlankWithComment>{};
    struct lineZero                    : pegtl::seq<bol, eol>{};
    struct lineEmpty                   : pegtl::sor<lineZero, lineBlank>{};
}  // namespace parr::rule
