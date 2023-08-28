#include <csignal>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <limits>

#include "prs.hpp"

using namespace std::string_literals;

// files

bool prs::base::LoadFile( const std::string& filename )
{
    UnloadFile();

    std::string content;

    if( !prs::LoadFile( filename, content ) )
        return false;

    GetInput()->load( content );
    GetInput()->name = filename;
    GetLexer()->setInputStream( GetInput() );
    GetTokens()->setTokenSource( GetLexer() );
    GetParser()->setTokenStream( GetTokens() );

    return true;
}

void prs::base::UnloadFile()
{
    LastParseTree = nullptr;

    PrintTrace( "UnloadFile=>NeedFill=true" );
    NeedFill = true;

    GetInput()->reset();
    GetInput()->load( nullptr, 0 );
    GetInput()->name.clear();
    GetLexer()->reset();
    GetTokens()->reset();
    GetParser()->reset();
}

// work

bool prs::base::Parse( antlr4::atn::PredictionMode mode /* = antlr4::atn::PredictionMode::LL */ )
{
    GetParser()->getInterpreter<antlr4::atn::ParserATNSimulator>()->setPredictionMode( mode );

    LastParseTree = nullptr;
    LastParseTree = RunParser();

    PrintTrace( "Parse=>NeedFill=false" );
    NeedFill = false;

    return LastParseTree && GetParser()->getNumberOfSyntaxErrors() == 0;
}

bool prs::base::ParseAdaptive()
{
    Ref<antlr4::ANTLRErrorStrategy> oldErrorHandler = GetParser()->getErrorHandler();

    // PrintTrace( "parser->removeErrorListeners()" );
    // GetParser()->removeErrorListeners();
    PrintTrace( "parser->setErrorHandler()" );
    GetParser()->setErrorHandler( std::make_shared<antlr4::BailErrorStrategy>() );

    bool oldTrace = GetParser()->isTrace();

    try
    {
        PrintTrace( "prediction=SLL" );
        PrintTrace( "--------------" );

        Parse( antlr4::atn::PredictionMode::SLL );
    }
    catch( const antlr4::ParseCancellationException& e )
    {
        PrintTrace( "parse tree="s + ( GetLastParseTree() ? "OK" : "NULL" ) );
        PrintTrace( "syntax errors="s + std::to_string( GetParser()->getNumberOfSyntaxErrors() ) );

        PrintTrace( "prediction=LL" );
        PrintTrace( "-------------" );
        PrintTrace( "reset=tokens,parser" );

        GetTokens()->reset();
        GetParser()->reset();

        PrintTrace( "ParseAdaptive=>NeedFill=true" );
        NeedFill = true;

        // PrintTrace( "parser->removeErrorListeners()" );
        // GetParser()->removeErrorListeners();
        // PrintTrace( "parser->addErrorListener()" );
        // GetParser()->addErrorListener( &antlr4::ConsoleErrorListener::INSTANCE );
        PrintTrace( "parser->setErrorHandler()" );
        GetParser()->setErrorHandler( oldErrorHandler );
        GetParser()->setTrace( oldTrace );

        Parse( antlr4::atn::PredictionMode::LL );
    }

    PrintTrace( "parse tree="s + ( GetLastParseTree() ? "OK" : "NULL" ) );
    PrintTrace( "syntax errors="s + std::to_string( GetParser()->getNumberOfSyntaxErrors() ) );

    return GetLastParseTree() && GetParser()->getNumberOfSyntaxErrors() == 0;
}

//
// diagnostics
//

antlr4::tree::ParseTree* prs::base::GetLastParseTree()
{
    return LastParseTree;
}

std::vector<std::string> prs::base::GetTokensVec( bool full /* = false */, bool insertSpace /* = false */, bool insertNewline /* = false */ )
{
    if( NeedFill )
    {
        GetTokens()->fill();

        PrintTrace( "GetTokensVec=>NeedFill=false" );
        NeedFill = false;
    }

    std::vector<std::string> result;
    bool                     lineStart = true;
    for( const auto& token : GetTokens()->getTokens() )
    {
        size_t      type = token->getType();
        std::string text = token->getText();
        std::string name = std::string( GetLexer()->getVocabulary().getSymbolicName( type ) );

        if( name.empty() )
            name += "<\"" + antlrcpp::escapeWhitespace( text, false ) + "\">";

        if( full )
        {
            name += ":index=" + std::to_string( token->getTokenIndex() );
            name += ",type=" + ( type == std ::numeric_limits<size_t>::max() ? "-1" : std::to_string( type ) );
            name += ",channel=" + std::to_string( token->getChannel() );
            name += ",file=" + static_cast<antlr4::ANTLRInputStream*>( token->getInputStream() )->name;
            name += ",line=" + std::to_string( token->getLine() );
            name += ",column=" + std::to_string( token->getCharPositionInLine() + 1 );
            // name += ",text=" + antlrcpp::escapeWhitespace( text, false );
        }

        if( insertSpace && !lineStart )
            result.push_back( " " );

        result.push_back( name );
        lineStart = false;

        // if current token is newline, insert one in output as well
        if( text == "\r\n" || text == "\n" )
        {
            if( insertNewline )
                result.push_back( "\n" );
            lineStart = true;
        }
    }

    return result;
}

void prs::base::PrintTokens( bool full /* = false */ )
{
    std::vector<std::string> tokens = GetTokensVec( full, !full, !full );

    for( const auto& token : tokens )
    {
        std::cout << token;
        if( full )
            std::cout << '\n';
    }

    if( !full )
        std::cout << '\n';
}

void prs::base::PrintTrace( const std::string& prefix, const std::string& message )
{
    if( !GetParser()->isTrace() || prefix.empty() || message.empty() )
        return;

    std::string indented = prefix;
    if( indented.size() < 7 )
        indented += std::string( 7 - indented.size(), ' ' );

    std::cout << indented << " " << message << '\n';
}

void prs::base::PrintTrace( const std::string& message )
{
    PrintTrace( "prs", message );
}

//
// utils
//

bool prs::LoadFile( const std::string& filename, std::string& content )
{
    content.clear();

    constexpr size_t read_size = 4096;
    std::ifstream    stream( filename, std::ios_base::in | std::ios_base::binary );
    stream.exceptions( std::ios_base::badbit );

    if( !stream )
        return false;

    std::string buf( read_size, '\0' );
    while( stream.read( &buf[0], read_size ) )
        content.append( buf, 0, stream.gcount() );
    content.append( buf, 0, stream.gcount() );

    return true;
}
