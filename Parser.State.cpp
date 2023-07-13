#include "Parser.hpp"

parr::Parser::State::State( parr::Parser& parser, parr::Parser::Tool& tool ) :
    Parser( parser ),
    Tool( tool ),
    Result( true )
{}

parr::Parser::State::~State()
{}
