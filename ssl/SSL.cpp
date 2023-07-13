#include <iostream>

#include "Parser.Tool.hpp"

#include "ssl/SSL.Rules.hpp"
#include "ssl/SSL.hpp"

namespace pegtl = TAO_PEGTL_NAMESPACE;

namespace parr::action::ssl
{
    /*
    template<typename Rule>
    struct control : pegtl::normal<Rule>
    {
        template<typename ParseInput, typename... States>
        static void success( const ParseInput&, States&&... )
        {
            std::string_view rule = pegtl::demangle<Rule>();

            if( !rule.starts_with( "parr::rule" ) )
                return;

            std::cout << "SUCCESS " << rule << std::endl;
            //        << "TEXT <" << input.current() << ">" << std::endl;
        }
    };
    */

    template<typename Rule>
    struct on : pegtl::nothing<Rule>
    {};

    std::string LastProcedureName;

    template<>
    struct on<rule::ssl::procedure::name>
    {
        template<typename ActionInput>
        static void apply( const ActionInput& input, Parser::State& )
        {
            std::cout << "APPLY procedure::name<" << input.string() << ">" << std::endl;
            LastProcedureName = input.string();
        }
    };

    template<>
    struct on<rule::ssl::procedure::declaration>
    {
        template<typename ActionInput>
        static void apply( const ActionInput& input, Parser::State& )
        {
            std::cout << "APPLY procedure::declaration<" << LastProcedureName << "><" << input.string() << ">" << std::endl;
        }
    };

    template<>
    struct on<rule::ssl::procedure::nop>
    {
        template<typename ActionInput>
        static void apply( const ActionInput& input, Parser::State& )
        {
            std::cout << "APPLY procedure::empty<" << input.string() << ">" << std::endl;
        }
    };

}  // namespace parr::action::ssl
/*
namespace parr::action::ssl
{
    template<typename ActionInput>
    void PrintActionInput( const ActionInput& input )
    {
        std::cout << "INPUT:position(" << input.position() << "),size:" << input.size() << std::endl;
    }

    template<typename Rule>
    struct on : pegtl::nothing<Rule>
    {};

    template<>
    struct on<rule::ssl::line::bol_>
    {
        template<typename ActionInput>
        static void apply( const ActionInput&, Parser::State& )
        {
            std::cout << "<- BOL" << std::endl;
        }
    };

    template<>
    struct on<rule::ssl::line::eol_>
    {
        template<typename ActionInput>
        static void apply( const ActionInput&, Parser::State& )
        {
            std::cout << "EOL ->" << std::endl;
        }
    };

    template<>
    struct on<rule::eof>
    {
        template<typename ActionInput>
        static void apply( const ActionInput&, Parser::State& state )
        {
            std::cout << "EOF =>" << std::endl;

            state.Result = true;
        }
    };

    template<>
    struct on<rule::commentShort>
    {
        template<typename ActionInput>
        static void apply( const ActionInput& input, Parser::State& )
        {
            std::cout << "CommentShort:<" << input.string() << ">" << std::endl;
        }
    };

    template<>
    struct on<rule::commentMedium>
    {
        template<typename ActionInput>
        static void apply( const ActionInput& input, Parser::State& )
        {
            std::cout << "CommentMedium<" << input.string() << ">" << std::endl;
        }
    };

    template<>
    struct on<rule::commentLong>
    {
        template<typename ActionInput>
        static void apply( const ActionInput& input, Parser::State& )
        {
            std::cout << "CommentLong:<" << input.string() << ">" << std::endl;
        }
    };

    template<>
    struct on<rule::ssl::procedureHead>  // called after matching [procedure * name * begin]
    {
        template<typename ActionInput>
        static void apply( const ActionInput& input, Parser::State& )
        {
            std::cout << "ProcedureHead<" << input.string() << ">" << std::endl;
        }
    };

    template<>
    struct on<rule::ssl::procedureEmpty>
    {
        template<typename ActionInput>
        static void apply( const ActionInput& input, Parser::State& )
        {
            PrintActionInput( input );
            std::cout << "ProcedureEmpty<" << input.string() << ">" << std::endl;
        }
    };

    template<>
    struct on<rule::ssl::procedureEmpty::name>
    {
        template<typename ActionInput>
        static void apply( const ActionInput& input, Parser::State& )
        {
            std::cout << "ProcedureEmpty::Name<" << input.string() << ">" << std::endl;
        }
    };
}  // namespace parr::action::ssl
*/
parr::SSL::SSL() {}

parr::SSL::~SSL() {}

void parr::SSL::Run( const Parser::RunMode& mode, pegtl::string_input<>& input, Parser::State& state )
{
    /*
    if( mode == Parser::RunMode::Parse )
    {
        if( !pegtl::parse<rule::ssl::rGlobalScope, action::ssl::on, action::ssl::control>( input, state ) )
            state.Result = false;

        return;
    }
    */

    run::Run<rule::ssl::rGlobalScope, action::ssl::on>( mode, input, state );
}
