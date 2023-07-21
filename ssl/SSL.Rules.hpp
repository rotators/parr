#pragma once

#include "Rules.hpp"

// clang-format-sh on

namespace parr::rule::ssl
{
    namespace pegtl = TAO_PEGTL_NAMESPACE;

    // general purpose
    // used by at least two rules

    struct charSemicolon          : pegtl::one<';'>{};

    struct stringBegin            : pegtl::string<'b', 'e', 'g', 'i', 'n'>{};
    struct stringEnd              : pegtl::string<'e', 'n', 'd'>{};
    struct stringImport           : pegtl::string<'i', 'm', 'p', 'o', 'r', 't'>{};
    struct stringLiteral          : pegtl::seq<pegtl::one<'"'>, pegtl::star<pegtl::seq<pegtl::not_one<'\\'>, any>>, pegtl::one<'"'>>{};

    struct blank                  : meta::eat<blanks::required>{};
    struct blankOpt               : meta::eat<blanks::optional>{};

    namespace operators
    {
        struct increase           : pegtl::two<'+'>{};
    }  // namespace operators

    // variables

    namespace variable
    {
        struct string             : pegtl::string<'v', 'a', 'r', 'i', 'a', 'b', 'l', 'e'>{};
        struct name               : pegtl::identifier{};  // TODO support special prefixes
        struct assign             : pegtl::sor<pegtl::string<':', '='>, pegtl::one<'='>>{};

        struct assignConstant     : pegtl::seq<assign, blankOpt, pegtl::digit>{};  // := 1

        struct declaration
        {
            struct localEmpty     : pegtl::seq<string, blank, name, blankOpt, charSemicolon>{};
            struct localAssign    : pegtl::seq<string, blank, name, blankOpt, assignConstant, blankOpt, charSemicolon>{};
            struct import         : pegtl::seq<stringImport, blank, localEmpty>{};  // imported variables cannot be assigned

            struct globalScope    : pegtl::sor<localEmpty, localAssign, import>{};
            struct procedureScope : pegtl::sor<localEmpty, localAssign>{};
        };

        struct increase           : pegtl::seq<name, blankOpt, operators::increase, blankOpt, charSemicolon>{};  // name++;

        struct globalScope        : declaration::globalScope{};
        struct procedureScope     : pegtl::sor<declaration::procedureScope, increase>{};
    }  // namespace variable

    namespace block
    {
        struct stringIf           : pegtl::string<'i', 'f'>{};
        struct stringThen         : pegtl::string<'t', 'h', 'e', 'n'>{};
        struct stringElse         : pegtl::string<'e', 'l', 's', 'e'>{};

        struct empty              : pegtl::seq<stringBegin, blankOpt, stringEnd>{};

        struct ifEmpty            : pegtl::seq<stringIf, pegtl::not_at<blank, stringThen>, pegtl::until<stringThen, any>, blank, empty>{};  // TODO

        struct x                  : pegtl::sor<empty, ifEmpty>{};
    }  // namespace block

    // procedures

    namespace procedure
    {
        struct string             : pegtl::string<'p', 'r', 'o', 'c', 'e', 'd', 'u', 'r', 'e'>{};
        struct name               : pegtl::identifier{};  // TODO support special prefixes

        struct head               : meta::eat<string, blank, name>{};
        struct arguments          : meta::eat<charParenOpen, blankOpt, charParenClose>{};  // \(\) or \(.*\)
        struct declaration        : meta::eat<charSemicolon>{};

        // used to inform actions that parser entered/left procedure scope
        // MUST match ONLY ONCE per procedure
        namespace scope
        {
            struct begin          : meta::eat<stringBegin>{};
            struct end            : meta::eat<stringEnd>{};
        }  // namespace scope

        // matches whole procedure body - including "begin" and "end" - AFTER it's parsed
        struct body               : pegtl::seq<scope::begin, pegtl::until<scope::end, pegtl::sor<blank, variable::procedureScope, block::x>>>{};

        struct check              : pegtl::seq<head, blankOpt, pegtl::opt<arguments>, blankOpt, pegtl::sor<declaration, body>>{};
    }  // namespace procedure

    // used as starting point
    struct GlobalScope            : pegtl::seq<pegtl::bof, bom::optional, pegtl::until<blanks::eof, pegtl::sor<blank, procedure::check, variable::globalScope>>>{};

}  // namespace parr::rule::ssl
