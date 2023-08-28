#include <cstdio>  // setvbuf
#include <filesystem>

#include "executable.hpp"

using namespace std::string_literals;

namespace
{
    const std::string OptionHelp = "help";
    const std::string OptionFile = "file";

    const std::string OptionTokens = "tokens";
    const std::string OptionTrace  = "trace";
    const std::string OptionTree   = "tree";

    cxxopts::Options     Options( "prs" );
    cxxopts::ParseResult OptionsParsed;
    bool                 OptionsParsedAlready = false;

    int    ArgC;
    char** ArgV;

    void Message( const std::string& type, const std::string& message )
    {
        if( type.empty() || message.empty() )
            return;

        std::string space = " ";

        if( message.front() == '[' || message.front() == '(' )
            space = "";

        std::cout << "[" << type << "]" << space << message << std::endl;
        std::cout << std::endl;
    }

    [[noreturn]] void ExitError( int status, const std::string& message = {}, const std::string& messageEx = {} )
    {
        prs::executable::Error( message );

        if( !messageEx.empty() )
            std::cout << messageEx << std::endl;

        std::exit( status );
    }

    void RunParserBefore( prs::base& base )
    {
        prs::executable::options::DiagnosticsTrace( base );
        prs::executable::options::DiagnosticsTokens( base );
    }

    void RunParserAfter( prs::base& base )
    {
        prs::executable::options::DiagnosticsTree( base );
    }
}  // namespace

void prs::executable::Init( int argc, char** argv, const std::string& program )
{
    std::setvbuf( stdout, nullptr, _IONBF, 0 );

    ArgC = argc;
    ArgV = argv;

    Options = cxxopts::Options( std::filesystem::path( ArgV[0] ).filename().string(), program );
    Options.add_options()( OptionHelp, "Usage" );
}

// TODO: clang installed on GHA runners cannot use std::source_location (v16.x required)
#if defined( __cpp_lib_source_location )
void prs::executable::Boop( const std::string& message /* = {} */, std::source_location src /* = std::source_location::current() */ )
{
    std::string messageFull = "["s + src.file_name() + ":" + std::to_string( src.line() ) /* + ":" + std::to_string( src.column() ) */ + "]" + "[" + src.function_name() + "]";

    if( !message.empty() )
        messageFull += " " + message;

    Message( "Boop", messageFull );
}
#else
void prs::executable::Boop( const std::string& message /* = {} */ )
{
    std::string messageFull = "[UnknownFile:0][UnknownFunction()]";

    if( !message.empty() )
        messageFull += " " + message;

    Message( "Boop", messageFull );
}
#endif

void prs::executable::Notice( const std::string& message )
{
    Message( "Notice", message );
}

void prs::executable::Warning( const std::string& message )
{
    Message( "Warning", message );
}

void prs::executable::Error( const std::string& message )
{
    Message( "Error", message );
}

//

bool prs::executable::RunParserWithOptions( prs::base& base )
{
    RunParserBefore( base );
    bool result = base.ParseAdaptive();
    RunParserAfter( base );

    return result;
}

bool prs::executable::RunParserWithOptions( prs::base& base, antlr4::atn::PredictionMode mode )
{
    RunParserBefore( base );
    bool result = base.Parse( mode );
    RunParserAfter( base );

    return result;
}

//

cxxopts::Options& prs::executable::options::Get()
{
    return Options;
}

cxxopts::ParseResult& prs::executable::options::GetParsed()
{
    if( !OptionsParsedAlready )
    {
        OptionsParsed        = Options.parse( ArgC, ArgV );
        OptionsParsedAlready = true;

        if( OptionsParsed.count( OptionHelp ) )
            ExitError( EXIT_SUCCESS );
    }

    return OptionsParsed;
}

void prs::executable::options::AddFile()
{
    Get().add_options()( OptionFile, "File", cxxopts::value<std::string>()->implicit_value( "" ) );
}

std::string prs::executable::options::File()
{
    std::string result;

    if( GetParsed().count( OptionFile ) )
    {
        result = GetParsed()[OptionFile].as<std::string>();

        if( result.empty() )
            ExitError( EXIT_FAILURE, "[Options] Missing argument for option <" + OptionFile + ">", Get().help() );
    }
    else
        ExitError( EXIT_FAILURE, "[Options] Missing option <" + OptionFile + ">", Get().help() );

    if( !std::filesystem::exists( result ) )
        ExitError( EXIT_FAILURE, "[Options] File does not exist <" + result + ">", Get().help() );

    return result;
}

//

void prs::executable::options::AddGroupDiagnostics()
{
    auto option = Options.add_options( "Diagnostics" );
    option( OptionTokens, "Tokens", cxxopts::value<std::string>()->implicit_value( "" ) );
    option( OptionTrace, "Trace" );
    option( OptionTree, "Tree" );
}

void prs::executable::options::DiagnosticsTokens( prs::base& base )  // Diagnostics() call
{
    if( !GetParsed().count( OptionTokens ) )
        return;

    base.PrintTokens( OptionsParsed[OptionTokens].as<std::string>() == "full" );
}

void prs::executable::options::DiagnosticsTrace( prs::base& base )  // Diagnostics() call
{
    if( !GetParsed().count( OptionTrace ) )
        return;

    base.GetParser()->setTrace( true );
}

void prs::executable::options::DiagnosticsTree( prs::base& base )  // manual call
{
    if( !GetParsed().count( OptionTree ) || !base.GetLastParseTree() )
        return;

    std::cout << base.GetLastParseTree()->toStringTree( true ) << std::endl;
}
