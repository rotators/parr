#include <csignal>
#include <cstdio>
#include <fstream>

#include "prs.hpp"

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
    GetInput()->reset();
    GetInput()->load( nullptr, 0 );
    GetInput()->name.clear();
    GetLexer()->reset();
    GetTokens()->reset();
    GetParser()->reset();
}

bool prs::base::Parse( bool trace /* = false */, antlr4::atn::PredictionMode mode /* = antlr4::atn::PredictionMode::SLL */ )
{
    if( trace )
        GetParser()->setTrace( trace );

    GetParser()->getInterpreter<antlr4::atn::ParserATNSimulator>()->setPredictionMode( mode );
    RunParser();

    return GetParser()->getNumberOfSyntaxErrors() == 0;
}

bool prs::base::ParseAdaptive( bool trace /* = false */ )
{
    // parser->removeErrorListeners();
    GetParser()->setErrorHandler( std::make_shared<antlr4::BailErrorStrategy>() );

    try
    {
        std::cout << "[[SLL]]" << std::endl;
        Parse( trace, antlr4::atn::PredictionMode::SLL );
    }
    catch( const antlr4::ParseCancellationException& e )
    {
        std::cout << "[[LL]]" << std::endl;
        GetTokens()->reset();
        GetParser()->reset();

        // parser->addErrorListener(&antlr4::ConsoleErrorListener::INSTANCE);
        GetParser()->setErrorHandler( std::make_shared<antlr4::DefaultErrorStrategy>() );
        Parse( trace, antlr4::atn::PredictionMode::LL );
    }

    return GetParser()->getNumberOfSyntaxErrors() == 0;
}

//

void prs::InitExecutable()
{
    std::setvbuf( stdout, nullptr, _IONBF, 0 );
}

bool prs::LoadFile( const std::string& filename, std::string& content )
{
    content.clear();

    constexpr size_t read_size = 4096;
    std::ifstream    stream( filename.data(), std::ios_base::in | std::ios_base::binary );
    stream.exceptions( std::ios_base::badbit );

    if( !stream )
        return false;

    std::string buf( read_size, '\0' );
    while( stream.read( &buf[0], read_size ) )
        content.append( buf, 0, stream.gcount() );
    content.append( buf, 0, stream.gcount() );

    return true;
}
