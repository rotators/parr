#pragma once

#include "Parser.hpp"

namespace parr
{
    namespace pegtl = TAO_PEGTL_NAMESPACE;

    class SSL : public Parser::Tool
    {
    public:
        SSL()          = default;
        virtual ~SSL() = default;

        virtual void Run( const Parser::RunMode& mode, pegtl::string_input<>& input, Parser::State& state ) override;
    };
}  // namespace parr
