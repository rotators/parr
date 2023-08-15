#pragma once

#include <functional>
#include <string>

#include <antlr4-runtime.h>

namespace prs
{
    class base
    {
    public:
        base() = default;
        virtual ~base() = default;
        
        virtual antlr4::Lexer* GetLexer() = 0;
        virtual antlr4::CommonTokenStream* GetTokens() = 0;
        virtual antlr4::Parser* GetParser() = 0;
        virtual void            RunParser() = 0;
    };

    template<typename LexerType, typename ParserType>
    class lib : public base
    {
    public:
        antlr4::ANTLRInputStream  Input;
        LexerType                 Lexer;
        antlr4::CommonTokenStream Tokens;
        ParserType                Parser;

        lib(const std::string& content, const std::string& /* name */ = {}) :
            Input(content), Lexer(&Input), Tokens(&Lexer),  Parser(&Tokens)
        {}

        virtual LexerType* GetLexer() override
        {
            return &Lexer;
        }

        virtual antlr4::CommonTokenStream* GetTokens()
        {
            return &Tokens;
        }

        virtual ParserType* GetParser() override
        {
            return &Parser;
        }

        virtual void RunParser() override
        {
            Parser.prs();
        }
    };

    void InitExecutable();

    std::string LoadFile(std::string_view filename);

    bool Parse(base& lib, bool trace = false, antlr4::atn::PredictionMode mode = antlr4::atn::PredictionMode::SLL);
    bool ParseAdaptive(base& lib, bool trace = false);
}  // namespace prs
