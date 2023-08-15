#include <cstdio>
#include <fstream>
#include <string>

#include "prs.hpp"

//

void prs::InitExecutable()
{
    std::setvbuf( stdout, nullptr, _IONBF, 0 );
}

std::string prs::LoadFile( std::string_view filename )
{
    std::string out;

    constexpr size_t read_size = 4096;
    std::ifstream stream( filename.data() );
    stream.exceptions( std::ios_base::badbit );

    if( !stream )
    {
        throw std::ios_base::failure( "file does not exist" );
        return out;
    }

    std::string buf( read_size, '\0' );
    while( stream.read( &buf[0], read_size ) )
    {
        out.append( buf, 0, stream.gcount() );
    }
    out.append( buf, 0, stream.gcount() );

    return out;
}

bool prs::Parse(base& lib, bool trace /* = false */, antlr4::atn::PredictionMode mode /* = antlr4::atn::PredictionMode::SLL */)
{
    if(trace)
        lib.GetParser()->setTrace(trace);

    lib.GetParser()->getInterpreter<antlr4::atn::ParserATNSimulator>()->setPredictionMode(mode);
    lib.RunParser();

    return lib.GetParser()->getNumberOfSyntaxErrors() == 0;
}

bool prs::ParseAdaptive(base& lib, bool trace /* = false */)
{
    antlr4::Parser* parser = lib.GetParser();

    //parser->removeErrorListeners();
    parser->setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());

    try
    {
        std::cout << "[[SLL]]" << std::endl;
        Parse(lib, trace, antlr4::atn::PredictionMode::SLL);
    }
    catch(const antlr4::ParseCancellationException& e)
    {
        std::cout << "[[LL]]" << std::endl;
        lib.GetTokens()->reset();
        parser->reset();

        //parser->addErrorListener(&antlr4::ConsoleErrorListener::INSTANCE);
        parser->setErrorHandler(std::make_shared<antlr4::DefaultErrorStrategy>());
        Parse(lib, trace, antlr4::atn::PredictionMode::LL);
    }

    return parser->getNumberOfSyntaxErrors() == 0;
}
