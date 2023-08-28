#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "executable.hpp"
#include "prs.hpp"
#include "prs.ssl.hpp"

int main( int argc, char** argv )
{
    prs::executable::Init( argc, argv, "SSL parser" );
    {
        prs::executable::options::AddFile();
        prs::executable::options::AddGroupDiagnostics();
    }

    prs::lib<prs::ssl::Lexer, prs::ssl::Parser> ssl;

    std::string filename = prs::executable::options::File();
    if( !ssl.LoadFile( filename ) )
    {
        prs::executable::Error( "File cannot be loaded <" + filename + ">" );
        return EXIT_FAILURE;
    }

    bool result = prs::executable::RunParserWithOptions( ssl );

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
