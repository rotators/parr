#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include <cxxopts.hpp>

#include "ssl/SSL.hpp"

int main( int argc, char** argv )
{
    parr::InitExecutable();

    auto options = cxxopts::Options( argv[0], "SSL parser" );
    {
        auto add = options.add_options();

        add( "help", "Usage" );
        //add( "verbose", "Verbose" );

        add( "analyze-rules", "?", cxxopts::value<std::string>()->default_value( "skip" )->implicit_value( "exit-on-error" ) );
        add( "file", "?", cxxopts::value<std::string>() );
        add( "run", "?", cxxopts::value<std::string>() );
    }

    auto userOptions = options.parse( argc, argv );

    if( userOptions.count( "help" ) )
    {
        std::cout << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    parr::ParserTool<parr::SSL> ssl;
    std::string                 contentName, content;

    if( userOptions.count( "analyze-rules" ) )
    {
        const std::string mode = userOptions["analyze-rules"].as<std::string>();

        if( mode != "skip" )
        {
            if( !ssl.Analyze() )
                return EXIT_FAILURE;

            if( mode == "exit-after" )
                return EXIT_SUCCESS;
        }
    }

    if( userOptions.count( "file" ) )
    {
        contentName = userOptions["file"].as<std::string>();

        if( !std::filesystem::exists( contentName ) )
        {
            std::cerr << "File does not exist: [" << contentName << std::string( "]" ) << std::endl;
            return EXIT_FAILURE;
        }

        content = ssl.LoadFile( contentName );
    }
    else
    {
        std::cerr << "Missing option: --file" << std::endl;
        return EXIT_FAILURE;
    }

    if( userOptions.count( "run" ) )
    {
        std::string run = userOptions["run"].as<std::string>();

        bool result = false;

        if( run == "coverage" )
            result = ssl.Coverage( content, contentName );
        else if( run == "parse" )
            result = ssl.Parse( content, contentName );
        else if( run == "parse-tree" )
            result = ssl.ParseTree( content, contentName );
        else if( run == "trace" )
            result = ssl.Trace( content, contentName );
        else if( run == "parse-error-trace" )
            result = ssl.ParseErrorTrace( content, contentName );
        else
        {
            result = false;
            std::cout << "Invalid option: --run=" << run << std::endl;
        }

        return result ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
