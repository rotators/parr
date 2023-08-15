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

            if( !rule.starts_with( "" ) )
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
    struct on<rule::blanks::spaces>
    {
        template<typename ActionInput>
        static void apply( const ActionInput& input, Parser::State& )
        {
            std::cout << "blanks::spaces<size:" << input.size() << ">" << std::endl;
        }
    };

    template<>
    struct on<rule::blanks::tabs>
    {
        template<typename ActionInput>
        static void apply( const ActionInput& input, Parser::State& )
        {
            std::cout << "blanks::tabs<size:" << input.size() << ">" << std::endl;
        }
    };

    template<>
    struct on<rule::blanks::eols>
    {
        template<typename ActionInput>
        static void apply( const ActionInput& input, Parser::State& )
        {
            std::cout << "blanks::eols<size:" << input.size() << ">" << std::endl;
        }
    };

    //

    template<>
    struct on<rule::ssl::procedure::name>
    {
        template<typename ActionInput>
        static void apply( const ActionInput& input, Parser::State& )
        {
            std::cout << "ssl::procedure::name<" << input.string() << ">" << std::endl;
            LastProcedureName = input.string();
        }
    };

    template<>
    struct on<rule::ssl::procedure::head>
    {
        template<typename ActionInput>
        static void apply( const ActionInput& input, Parser::State& )
        {
            std::cout << "ssl::procedure::head<" << input.string() << ">" << std::endl;
        }
    };

    template<>
    struct on<rule::ssl::procedure::arguments>
    {
        template<typename ActionInput>
        static void apply( const ActionInput& input, Parser::State& )
        {
            std::cout << "ssl::procedure::arguments<" << input.string() << ">" << std::endl;
        }
    };

    template<>
    struct on<rule::ssl::procedure::declaration>
    {
        template<typename ActionInput>
        static void apply( const ActionInput& input, Parser::State& )
        {
            std::cout << "ssl::procedure::declaration<" << LastProcedureName << "><" << input.string() << ">" << std::endl;
        }
    };

    template<>
    struct on<rule::ssl::procedure::scope::begin>
    {
        template<typename ActionInput>
        static void apply( const ActionInput&, Parser::State& )
        {
            std::cout << "ssl::procedure::scope::begin" << std::endl;
        }
    };

    template<>
    struct on<rule::ssl::procedure::scope::end>
    {
        template<typename ActionInput>
        static void apply( const ActionInput&, Parser::State& )
        {
            std::cout << "ssl::procedure::scope::end" << std::endl;
        }
    };

}  // namespace parr::action::ssl

void parr::SSL::Run( const Parser::RunMode& mode, pegtl::string_input<>& input, Parser::State& state )
{
    run::Run<rule::ssl::GlobalScope, action::ssl::on>( mode, input, state );
}
