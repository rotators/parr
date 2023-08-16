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

    auto options = cxxopts::Options( std::filesystem::path( argv[0] ).filename().string(), "SSL parser" );
    {
        auto add = options.add_options();

        add( "help", "Usage" );
        // add( "verbose", "Verbose" );

        add( "file", "?", cxxopts::value<std::string>() );
    }

    auto usage = [&options]( int status, const std::string& error = {} ) {
        if( !error.empty() )
        {
            std::cout << error << std::endl;
            std::cout << std::endl;
        }
        std::cout << options.help() << std::endl;

        return status;
    };

    auto userOptions = options.parse( argc, argv );
    if( userOptions.count( "help" ) )
        return usage( EXIT_SUCCESS );

    std::string filename, content;

    if( userOptions.count( "file" ) )
    {
        filename = userOptions["file"].as<std::string>();

        if( filename.empty() )
            return usage( EXIT_FAILURE, "Missing filename for option: --file" );
    }
    else
        return usage( EXIT_FAILURE, "Missing option: --file" );

    prs::lib<prs::ssl::Lexer, prs::ssl::Parser> ssl;  //(content, filename);

    if( !ssl.LoadFile( filename ) )
    {
        std::cerr << "File cannot be loaded: [" << filename << "]" << std::endl;
        return EXIT_FAILURE;
    }

    antlr4::DiagnosticErrorListener diagnostics;
    ssl.GetParser()->removeErrorListeners();
    ssl.GetParser()->addErrorListener( &diagnostics );

    bool result = ssl.Parse( true );

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
