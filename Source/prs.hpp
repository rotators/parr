#pragma once

#include <functional>
#include <string>
#include <vector>

#include <antlr4-runtime.h>

namespace prs
{
    class base
    {
    private:
        antlr4::tree::ParseTree* LastParseTree = nullptr;
        bool                     NeedFill      = true;

    public:
        base()              = default;
        base( const base& ) = delete;
        base( base&& )      = delete;
        virtual ~base()     = default;

        base& operator=( const base& ) = delete;
        base& operator=( base&& )      = delete;

    public:  // lib
        virtual antlr4::ANTLRInputStream*  GetInput()  = 0;
        virtual antlr4::Lexer*             GetLexer()  = 0;
        virtual antlr4::CommonTokenStream* GetTokens() = 0;
        virtual antlr4::Parser*            GetParser() = 0;
        virtual antlr4::tree::ParseTree*   RunParser() = 0;

    public:  // files
        bool LoadFile( const std::string& filename );
        void UnloadFile();

    public:  // work
        bool Parse( antlr4::atn::PredictionMode mode = antlr4::atn::PredictionMode::LL );
        bool ParseAdaptive();

    public:  // diagnostics
        antlr4::tree::ParseTree* GetLastParseTree();
        std::vector<std::string> GetTokensVec( bool full = false, bool insertSpace = false, bool insertNewline = false );
        void                     PrintTokens( bool full = false );
        void                     PrintTrace( const std::string& prefix, const std::string& message );

    protected:
        void PrintTrace( const std::string& message );
    };

    template<typename LexerType, typename ParserType>
    class lib final : public base
    {
    private:
        antlr4::ANTLRInputStream  Input;
        LexerType                 Lexer;
        antlr4::CommonTokenStream Tokens;
        ParserType                Parser;

    public:
        lib() :
            Input(), Lexer( &Input ), Tokens( &Lexer ), Parser( &Tokens ){};

    public:
        virtual antlr4::ANTLRInputStream*  GetInput() override { return &Input; }
        virtual LexerType*                 GetLexer() override { return &Lexer; }
        virtual antlr4::CommonTokenStream* GetTokens() override { return &Tokens; }
        virtual ParserType*                GetParser() override { return &Parser; }
        virtual antlr4::tree::ParseTree*   RunParser() override { return Parser.prs(); }
    };

    // utils

    bool LoadFile( const std::string& filename, std::string& content );
}  // namespace prs
