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
        //add( "verbose", "Verbose" );

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
    {
        contentName = userOptions["file"].as<std::string>();

        if( !std::filesystem::exists( contentName ) )
        {
            std::cerr << "File does not exist: [" << contentName << std::string( "]" ) << std::endl;
            return EXIT_FAILURE;
        }

        content = prs::LoadFile( contentName );
    }
    else
    {
        std::cerr << "Missing option: --file" << std::endl;
        return EXIT_FAILURE;
    }

    bool result = true;

    prs::lib<prs::ssl::Lexer, prs::ssl::Parser> ssl(content, contentName);

    antlr4::DiagnosticErrorListener diagnostics;
    ssl.Parser.removeErrorListeners();
    ssl.Parser.addErrorListener(&diagnostics);

    result = prs::Parse(ssl, true);

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
