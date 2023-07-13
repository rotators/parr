#pragma once

#include <tao/pegtl.hpp>

namespace parr::rule
{
    namespace pegtl = TAO_PEGTL_NAMESPACE;

    // aliases

    using any = pegtl::any;

    // clang-format off

    namespace meta
    {
        // catch between X and Y
        // match X, ignore Y
        template<typename X, typename Y>
        struct XtoYexcludeY : pegtl::until<pegtl::at<Y>, X>{};
    }

    //

    using charParenOpen               = pegtl::one<'('>;
    using charParenClose              = pegtl::one<')'>;

    // newlines

    using charNewlineN                = pegtl::one<'\n'>;
    using charNewlineR                = pegtl::one<'\r'>;

    struct bol                        : pegtl::bol{};

    struct eof                        : pegtl::eof{};
    struct eolDos                     : pegtl::seq<charNewlineR, charNewlineN>{};
    struct eolUnix                    : charNewlineN{};
    struct eol                        : pegtl::sor<eolUnix,eolDos>{};

    struct eofl                       : pegtl::sor<eof,eol>{};
    struct eolf                       : pegtl::sor<eol,eof>{};

    // comments
    //
    //  comentShort    match anything between string "//" and EOL/EOF
    //                 EOL/EOF is NOT included in match
    //  commentMedium  match anything between string "/* and "*/"
    //                 NO match if EOL is found before
    //  commentLong    match anything between string "/*" and "*/"
    //                 MUST be checked AFTER [commentMedium]

    using stringCommentShort          = pegtl::two<'/'>;
    using stringCommentLongStart      = pegtl::ascii::string<'/','*'>;
    using stringCommentLongEnd        = pegtl::ascii::string<'*','/'>;

    struct commentShort                : pegtl::seq<stringCommentShort, pegtl::until<pegtl::at<eolf>, any>>{}; //meta::XtoYexcludeY<any, eolf>>{};
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

    // lines

    struct lineBlankWithComment      : pegtl::seq<bol, blankWithComment, eol>{};
    struct lineBlankWithoutComment   : pegtl::seq<bol, blankWithoutComment, eol>{};
    struct lineBlank                  : pegtl::sor<lineBlankWithoutComment, lineBlankWithComment>{};
    struct lineZero                   : pegtl::seq<bol, eol>{};
    struct lineEmpty                  : pegtl::sor<lineZero, lineBlank>{};

    // clang-format on
}  // namespace parr::rule
