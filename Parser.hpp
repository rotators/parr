#pragma once

#include <string>
#include <unordered_map>

#include <tao/pegtl.hpp>

namespace parr
{
    namespace pegtl = TAO_PEGTL_NAMESPACE;

    void InitExecutable();

    class Parser
    {
    public:
        Parser();
        virtual ~Parser();

    public:
        bool AnalyzeRules();

        enum class RunMode : uint8_t
        {
            Analyze,
            Coverage,
            Parse,
            Trace,
            ParseTree,

            // meta
            ParseErrorTrace  // run Parse, (on error) run ParseTrace
        };

        struct State;

        class Tool
        {
        public:
            virtual void Run( const RunMode& mode, pegtl::string_input<>& input, State& state ) = 0;
        };

        struct State
        {
            const parr::Parser&       Parser;
            const parr::Parser::Tool& Tool;

            bool Result;

            State( parr::Parser& parser, parr::Parser::Tool& tool );
            virtual ~State();

            State()          = delete;
            State( State& )  = delete;
            State( State&& ) = delete;
        };

        std::string LoadFile( std::string_view filename );

        bool Analyze( Tool& tool );
        bool Coverage( Tool& tool, const std::string& content, const std::string& contentName = {} );
        bool Parse( Tool& tool, const std::string& content, const std::string& contentName = {} );
        bool ParseTree( Tool& tool, const std::string& content, const std::string& contentName = {} );
        bool Trace( Tool& tool, const std::string& content, const std::string& contentName = {} );
        bool ParseErrorTrace( Tool& tool, const std::string& content, const std::string& contentName = {} );

    protected:
        bool Run( Tool& tool, const RunMode& mode, const std::string& content = {}, const std::string& contentName = {} );
    };

    template<typename Tool>
    class ParserTool : public Parser, private Tool
    {
    public:
        ParserTool(){};
        virtual ~ParserTool(){};

        bool Analyze() { return Parser::Analyze( *this ); }
        bool Coverage( const std::string& content, const std::string& contentName = {} ) { return Parser::Coverage( *this, content, contentName ); }
        bool Parse( const std::string& content, const std::string& contentName = {} ) { return Parser::Parse( *this, content, contentName ); }
        bool ParseTree( const std::string& content, const std::string& contentName = {} ) { return Parser::ParseTree( *this, content, contentName ); }
        bool Trace( const std::string& content, const std::string& contentName = {} ) { return Parser::Trace( *this, content, contentName ); }
        bool ParseErrorTrace( const std::string& content, const std::string& contentName = {} ) { return Parser::ParseErrorTrace( *this, content, contentName ); }
    };

}  // namespace parr
