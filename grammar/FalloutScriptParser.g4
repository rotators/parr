parser grammar FalloutScriptParser;

options
{
    tokenVocab = FalloutScriptLexer;
}

prs: ssl ;

ssl: global_scope* EOF ;

global_scope
    : blank+
    | variableDeclaration
    | procedureDeclaration
    | variableImport
    | procedureImport
    | procedureBody 
    ;

procedure_scope
    : blank+
    | variableDeclaration
    | variableOp
    | block
    ;

//

procedureBody
    : procedureBegin blank+ procedureEnd
    | procedureBegin procedure_scope+ procedureEnd
    ;

procedureBegin
    : procedureHead blank* procedureArguments blank* BEGIN
    | procedureHead blank+ BEGIN
    ;

procedureEnd
    : END
    ;

//

procedureImport
    : IMPORT blank+ procedureDeclaration
    ;

procedureDeclaration
    : procedureHead blank* arguments=procedureArguments blank* SEMICOLON # procedureDeclarationArguments
    | procedureHead blank* SEMICOLON                                     # procedureDeclarationEmpty
    ;

procedureArguments
    : PAREN_OPEN blank* PAREN_CLOSE
    ;

procedureHead : PROCEDURE blank+ name=IDENTIFIER ;

//

variableImport
    : IMPORT blank+ variableHead blank* SEMICOLON
    ;

variableDeclaration
    : variableHead blank* (OP_ASSIGN2 | OP_ASSIGN1) blank* value=NUMBER blank* SEMICOLON
    | variableHead blank* SEMICOLON
    ;

variableOp
    : name=IDENTIFIER blank* OP_INCREASE blank* SEMICOLON # variableOpIncrease
    ;

variableHead : VARIABLE blank+ name=IDENTIFIER;

//

ifCondition
    : bool=(TRUE | FALSE)
    | name=IDENTIFIER
    ;
//

blockHead
    : IF blank* PAREN_OPEN? ifCondition PAREN_CLOSE? blank* THEN # blockkHeadIf // WIP
    ;

blockBody
    : BEGIN blank+ END           # blockBodyEmpty
    | BEGIN procedure_scope+ END # blockBodyScope
    ;

block
    : (blockHead blank+)? blockBody
    ;

//

blank         : ( spaces | tabs | eol+ | comment ) ;
spaces        : SPACE+ ;
tabs          : TAB+ ;
eol           : EOL_DOS | EOL_UNIX ;
comment       : commentShort | commentMedium | commentLong;
commentShort  : COMMENT_SHORT ;
commentMedium : COMMENT_MEDIUM ;
commentLong   : COMMENT_LONG ;
