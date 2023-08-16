#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include <cxxopts.hpp>

#include "prs.hpp"
#include "prs.ssl.hpp"

int main( int argc, char** argv )
{
    prs::InitExecutable();

    auto options = cxxopts::Options( argv[0], "SSL parser" );
    {
        auto add = options.add_options();

        add( "help", "Usage" );
        // add( "verbose", "Verbose" );

        add( "file", "?", cxxopts::value<std::string>() );
    }

    auto userOptions = options.parse( argc, argv );

    std::string contentName, content;

    if( userOptions.count( "help" ) )
    {
        std::cout << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    if( userOptions.count( "file" ) )
        contentName = userOptions["file"].as<std::string>();
    else
    {
        std::cerr << "Missing option: --file" << std::endl;
        return EXIT_FAILURE;
    }

    bool result = true;

    prs::lib<prs::ssl::Lexer, prs::ssl::Parser> ssl;  //(content, contentName);

    if( !ssl.LoadFile( contentName ) )
    {
        std::cerr << "File cannot be loaded: [" << contentName << "]" << std::endl;
        return EXIT_FAILURE;
    }

    antlr4::DiagnosticErrorListener diagnostics;
    ssl.GetParser()->removeErrorListeners();
    ssl.GetParser()->addErrorListener( &diagnostics );

    result = ssl.Parse( true );

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
