#include <cstdio>  // std::setvbuf
#include <fstream>
#include <string>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>

#include "Parser.hpp"

namespace pegtl = TAO_PEGTL_NAMESPACE;

parr::Parser::Parser()
{
}

parr::Parser::~Parser()
{
}

void parr::InitExecutable()
{
    std::setvbuf( stdout, nullptr, _IONBF, 0 );
}

std::string parr::Parser::LoadFile( std::string_view filename )
{
    auto out = std::string();

    constexpr auto read_size = std::size_t( 4096 );
    auto           stream    = std::ifstream( filename.data() );
    stream.exceptions( std::ios_base::badbit );

    if( !stream )
    {
        throw std::ios_base::failure( "file does not exist" );
        return out;
    }

    auto buf = std::string( read_size, '\0' );
    while( stream.read( &buf[0], read_size ) )
    {
        out.append( buf, 0, stream.gcount() );
    }
    out.append( buf, 0, stream.gcount() );

    return out;
}

bool parr::Parser::Analyze( Tool& tool )
{
    return Run( tool, RunMode::Analyze );
}

bool parr::Parser::Coverage( Tool& tool, const std::string& content, const std::string& contentName )
{
    return Run( tool, RunMode::Coverage, content, contentName );
}

bool parr::Parser::Parse( Tool& tool, const std::string& content, const std::string& contentName )
{
    return Run( tool, RunMode::Parse, content, contentName );
}

bool parr::Parser::ParseTree( Tool& tool, const std::string& content, const std::string& contentName )
{
    return Run( tool, RunMode::ParseTree, content, contentName );
}

bool parr::Parser::Trace( Tool& tool, const std::string& content, const std::string& contentName )
{
    return Run( tool, RunMode::Trace, content, contentName );
}

bool parr::Parser::ParseErrorTrace( Tool& tool, const std::string& content, const std::string& contentName )
{
    return Run( tool, RunMode::ParseErrorTrace, content, contentName );
}

bool parr::Parser::Run( Tool& tool, const RunMode& mode, const std::string& content /* = {} */, const std::string& contentName /* = {} */ )
{
    State                      state( *this, tool );
    const std::string          dummyString = {};
    static pegtl::string_input dummyInput( "", "(dummy)" );

    if( mode == RunMode::Analyze )
        tool.Run( mode, dummyInput, state );
    else
    {
        pegtl::string_input input( content, contentName );

        tool.Run( ( mode == RunMode::ParseErrorTrace ? RunMode::Parse : mode ), input, state );

        if( !state.Result )
        {
        }

        if( mode == RunMode::ParseErrorTrace && !state.Result )
            return Run( tool, RunMode::Trace, content, contentName );
        else if( !state.Result )
        {
            std::cout << "PARSE FAIL" << std::endl;
            const auto position = input.position();
            std::cout
              << input.line_at( position ) << std::endl
              << std::string( position.column, '-' ) << '^' << std::endl;
        }
    }
    return state.Result;
}
