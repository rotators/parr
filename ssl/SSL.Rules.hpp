#pragma once

#include "Rules.hpp"

// clang-format-sh on

namespace parr::rule::ssl
{
    namespace pegtl = TAO_PEGTL_NAMESPACE;

    // general purpose
    // used by at least two rules

    using charSemicolon          = pegtl::one<';'>;

    using stringBegin            = pegtl::string<'b', 'e', 'g', 'i', 'n'>;
    using stringEnd              = pegtl::string<'e', 'n', 'd'>;
    using stringImport           = pegtl::string<'i', 'm', 'p', 'o', 'r', 't'>;
    using stringLiteral          = pegtl::seq<pegtl::one<'"'>, pegtl::star<pegtl::seq<pegtl::not_one<'\\'>, any>>, pegtl::one<'"'>>;

    using blank                  = blankWithComment;
    using blankOpt               = blankWithCommentOpt;

    namespace meta
    {
        template<typename String, typename Name>
        using declarationLong    = pegtl::seq<String, pegtl::pad<Name, blank>, charSemicolon>;
    }

    struct operators
    {
        using increase           = pegtl::two<'+'>;
    };

    // variables

    struct variable
    {
        using string             = pegtl::string<'v', 'a', 'r', 'i', 'a', 'b', 'l', 'e'>;
        using name               = pegtl::identifier;  // TODO support special prefixes
        using assign             = pegtl::sor<pegtl::string<':', '='>, pegtl::one<'='>>;

        struct assignConstant    : pegtl::seq<assign, blankOpt, pegtl::digit>{};  // := 1

        struct declaration
        {
            struct localEmpty    : pegtl::seq<string, blank, name, blankOpt, charSemicolon>{};
            struct localAssign   : pegtl::seq<string, blank, name, blankOpt, assignConstant, blankOpt, charSemicolon>{};
            struct import        : pegtl::seq<stringImport, blank, localEmpty>{};  // imported variables cannot be assigned

            using globalScope    = pegtl::sor<localEmpty, localAssign, import>;
            using procedureScope = pegtl::sor<localEmpty, localAssign>;
        };

        struct increase          : pegtl::seq<name, blankOpt, operators::increase, blankOpt, charSemicolon>{};  // name++;

        struct globalScope       : declaration::globalScope{};
        struct procedureScope    : pegtl::sor<declaration::procedureScope, increase>{};
    };

    struct block
    {
        using stringIf           = pegtl::string<'i', 'f'>;
        using stringThen         = pegtl::string<'t', 'h', 'e', 'n'>;
        using stringElse         = pegtl::string<'e', 'l', 's', 'e'>;

        struct empty             : pegtl::seq<stringBegin, blankOpt, stringEnd>{};

        struct ifEmpty           : pegtl::seq<stringIf, pegtl::not_at<blank, stringThen>, pegtl::until<stringThen, any>, blank, empty>{};  // TODO

        struct x                 : pegtl::sor<empty, ifEmpty>{};
    };

    // procedures

    struct procedure
    {
        using string             = pegtl::string<'p', 'r', 'o', 'c', 'e', 'd', 'u', 'r', 'e'>;
        using name               = pegtl::identifier;

        using proc               = pegtl::seq<string, blank, name>;

        struct arg
        {
            struct empty         : pegtl::sor<pegtl::seq<charParenOpen, charParenClose>, pegtl::seq<charParenOpen, blankOpt, charParenClose>>{};  // \(\) or \(.*\)
        };

        struct declaration
        {
            struct empty         : pegtl::seq<proc, blankOpt, charSemicolon>{};
            struct argEmpty      : pegtl::seq<proc, blankOpt, arg::empty, blankOpt, charSemicolon>{};

            struct x             : pegtl::sor<declaration::empty, declaration::argEmpty>{};
        };

        // regular procedures
        struct arguments
        {
            struct none          : pegtl::seq<proc, pegtl::at<blank, stringBegin>>{};  // procedure name begin
            struct empty         : pegtl::seq<proc, blankOpt, arg::empty, pegtl::at<blankOpt, stringBegin>>{};  // procedure name()begin

            // avoid including blanks in rules above
            struct x             : pegtl::sor<pegtl::seq<arguments::none, blank>, pegtl::seq<arguments::empty, blankOpt>>{};
        };

        // empty procedures doing nothing
        // matches whole procedures whole body - including "begin" and "end" - scope::begin/end rules are NOT used
        struct nop
        {
            struct none          : pegtl::seq<arguments::none, stringBegin, blank, stringEnd>{};  // procedure name begin end
            struct empty         : pegtl::seq<arguments::empty, stringBegin, blankOpt, stringEnd>{};  // procedure name()begin end

            struct x             : pegtl::sor<nop::none, nop::empty>{};
        };

        // used to inform actions that parser entered/left procedure scope
        // MUST match ONLY ONCE per procedure
        struct scope
        {
            struct begin         : stringBegin{};
            struct end           : stringEnd{};
        };

        // matches whole procedure body - including "begin" and "end" - AFTER it's parsed
        struct body              : pegtl::seq<scope::begin, pegtl::until<scope::end, pegtl::sor<eol, blank, variable::procedureScope, block::x>>>{};

        struct x_at              : pegtl::at<pegtl::opt<stringImport, blank>, string, blank, name, pegtl::opt<blankOpt, arg::empty>>{};
        struct x                 : pegtl::seq<x_at, pegtl::sor<declaration::x, nop::x, pegtl::seq<arguments::x, body>>>{};
    };

    // lines
    // used for quick/smooth/optimistic matching
    struct line                  : pegtl::sor<lineEmpty>
    {
        using bol_               = bol;
        using eol_               = eol;
    };

    // r*
    // used as starting point
    struct rGlobalScope          : pegtl::seq<pegtl::bof, pegtl::opt<bom::check>, pegtl::until<eof, pegtl::sor<eol, blank, procedure::x, variable::globalScope>>>{};

}  // namespace parr::rule::ssl
