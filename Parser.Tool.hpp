#pragma once

#include <string>

#undef __cpp_exceptions
#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/coverage.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>
#include <tao/pegtl/contrib/parse_tree_to_dot.hpp>
#include <tao/pegtl/contrib/print.hpp>
#include <tao/pegtl/contrib/print_coverage.hpp>
#include <tao/pegtl/contrib/trace.hpp>

#include "Parser.hpp"

namespace parr::run
{
    namespace pegtl = TAO_PEGTL_NAMESPACE;

    template<typename Rule>
    void Analyze( pegtl::string_input<>&, Parser::State& state )
    {
        // https://stackoverflow.com/a/55401199/11998612
        // result.emplace( std::piecewise_construct, std::forward_as_tuple( std::string( pegtl::demangle<R>() ) ), std::forward_as_tuple( issues, str.str() ) );

        if( pegtl::analyze<Rule>( 1 ) > 0 )
            state.Result = false;
    }

    template<typename Rule, template<typename> class Action>
    void Coverage( pegtl::string_input<>& input, Parser::State& state )
    {
        pegtl::coverage_result result;
        if( !pegtl::coverage<Rule, Action>( input, result, state ) )
        {
            state.Result = false;
            return;
        }

        std::cerr << result << std::endl;
    }

    template<typename Rule, template<typename> class Action>
    void Parse( pegtl::string_input<>& input, Parser::State& state )
    {
        if( !pegtl::parse<Rule, Action>( input, state ) )
            state.Result = false;
    }

    template<typename Rule, template<typename> class Action>
    void ParseTree( pegtl::string_input<>& input, Parser::State& state )
    {
        auto root = pegtl::parse_tree::parse<Rule, pegtl::parse_tree::internal::store_all, Action>( input, state );
        if( !root )
        {
            state.Result = false;
            return;
        }
        pegtl::parse_tree::print_dot( std::cerr, *root );
    }

    template<typename Rule, template<typename> class Action>
    void Trace( pegtl::string_input<>& input, Parser::State& state )
    {
        //pegtl::string_input input( content, contentName );

        using tracerTraits = pegtl::tracer_traits<true, false, true, 2, 5>;  // HideInternal, UseColor, PrintSourceLine, IndentIncrement, InitialIndent
        pegtl::tracer<tracerTraits> tracer( input );

        if( !tracer.parse<Rule, Action>( input, state ) )
            state.Result = false;
    }

    template<typename Rule, template<typename> class Action>
    void Run( const Parser::RunMode& mode, pegtl::string_input<>& input, Parser::State& state )
    {
        switch( mode )
        {
            case Parser::RunMode::Analyze:
                Analyze<Rule>( input, state );
                break;
            case Parser::RunMode::Coverage:
                Coverage<Rule, Action>( input, state );
                break;
            case Parser::RunMode::Parse:
                Parse<Rule, Action>( input, state );
                break;
            case Parser::RunMode::ParseTree:
                ParseTree<Rule, Action>( input, state );
                break;
            case Parser::RunMode::Trace:
                Trace<Rule, Action>( input, state );
                break;

            // silence -Wswitch
            // modes below are never passed to Parser::Tool
            case Parser::RunMode::ParseErrorTrace:
                break;
        }
    }

}  // namespace parr::run
