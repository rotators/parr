#pragma once

#include <tao/pegtl.hpp>

#include "Rules.hpp"

namespace parr::rule::ssl
{
    namespace pegtl = TAO_PEGTL_NAMESPACE;

    // clang-format off

    // general purpose
    // used by at least two rules

    using charSemicolon                = pegtl::one<';'>;

    using stringBegin                  = pegtl::string<'b', 'e', 'g', 'i', 'n'>;
    using stringEnd                    = pegtl::string<'e', 'n', 'd'>;
    using stringLiteral                = pegtl::seq<pegtl::one<'"'>, pegtl::star<pegtl::seq<pegtl::not_one<'\\'>, any>>, pegtl::one<'"'>>;

    using blank                        = blankWithComment;
    using blankOpt                     = blankWithCommentOpt;

    namespace meta
    {
        template<typename String, typename Name>
        using declarationLong          = pegtl::seq<String, pegtl::pad<Name, blank>, charSemicolon>;
    }

    // variables

    using stringVariable               = pegtl::string<'v', 'a', 'r', 'i', 'a', 'b', 'l', 'e'>;

    struct variableName                : pegtl::identifier{};
    struct variableDeclaration         : meta::declarationLong<stringVariable, variableName>{};
    struct variable                    : pegtl::sor<variableDeclaration>{}; // TODO

    // procedures

    struct procedure
    {
        using string                   = pegtl::string<'p', 'r', 'o', 'c', 'e', 'd', 'u', 'r', 'e'>;
        using name                     = pegtl::identifier;

        using proc                     = pegtl::seq<string, blank, name>;

        struct arg
        {
            struct empty               : pegtl::sor<pegtl::seq<charParenOpen,charParenClose>,pegtl::seq<charParenOpen, blankOpt, charParenClose>>{}; // \(\) or \(.*\)
        };

        struct declaration
        {
            //struct empty             : meta::declarationLong<string, name>{}; // pegtl::seq<stringProcedure, blankLong, procedureName, blankLong, charSemicolon> {};
            struct empty               : pegtl::seq<proc, blankOpt, charSemicolon>{};
            struct argEmpty            : pegtl::seq<proc, blankOpt, arg::empty, blankOpt, charSemicolon>{};

            struct x                   : pegtl::sor<declaration::empty, declaration::argEmpty>{};
        };

        struct end                    : stringEnd{};
        struct beginEnd               : pegtl::seq<stringBegin, blank, stringEnd>{};

        struct arguments
        {
            struct none               : pegtl::seq<proc, blank, stringBegin>{};                          // procedure name begin
            struct empty              : pegtl::seq<proc, blankOpt, arg::empty, blankOpt, stringBegin>{}; // procedure name()begin

            struct x                  : pegtl::sor<arguments::none, arguments::empty>{};
        };

        struct nop
        {
            struct none               : pegtl::seq<arguments::none, blank, stringEnd>{};     // procedure name begin end
            struct empty              : pegtl::seq<arguments::empty, blankOpt, stringEnd>{}; // procedure name()begin end

            struct x                  : pegtl::sor<nop::none,nop::empty>{};
        };

        struct x                      : pegtl::sor<declaration::x, nop::x, arguments::x>{};
    };

    // lines
    // used for quick/smooth/optimistic matching 
    struct line                       : pegtl::sor<lineEmpty>
    {
        using bol_ = bol;
        using eol_ = eol;
    };

    // r*
    // used as starting point
    struct rGlobalScope               : pegtl::until<eof, pegtl::sor<eol, blank, procedure::x, variable>> {};

    // clang-format on
}  // namespace parr::rule::ssl
