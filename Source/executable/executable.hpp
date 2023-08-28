#pragma once

#include <source_location>
#include <string>

#include <cxxopts.hpp>

#include "prs.hpp"

namespace prs::executable
{
    void Init( int argc, char** argv, const std::string& program );

// TODO: clang installed on GHA runners cannot use std::source_location (v16.x required)
#if defined( __cpp_lib_source_location )
    void Boop( const std::string& message = {}, std::source_location src = std::source_location::current() );
#else
    void Boop( const std::string& message = {} );
#endif

    void Notice( const std::string& message );
    void Warning( const std::string& message );
    void Error( const std::string& message );

    bool RunParserWithOptions( prs::base& base );

    // same as RunParser(base), but uses user-defined prediction mode
    bool RunParserWithOptions( prs::base& base, antlr4::atn::PredictionMode mode );
}  // namespace prs::executable

// NOTE: most of option functions might call std::exit() down the line,
//       if there's something wrong with user input
namespace prs::executable::options
{
    cxxopts::Options&     Get();
    cxxopts::ParseResult& GetParsed();

    // general

    void        AddFile();
    std::string File();

    // diagnostics

    void AddGroupDiagnostics();
    void DiagnosticsTokens( prs::base& base );
    void DiagnosticsTrace( prs::base& base );
    void DiagnosticsTree( prs::base& base );
}  // namespace prs::executable::options
