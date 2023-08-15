lexer grammar FalloutScriptLexer;

BEGIN     : 'begin' ;
DO        : 'do' ;
END       : 'end' ;
IF        : 'if' ;
IMPORT    : 'import' ;
PROCEDURE : 'procedure' ;
THEN      : 'then' ;
VARIABLE  : 'variable' ;
WHILE     : 'while' ;

COMMENT_SHORT       : COMMENT_SHORT_PREFIX ~[\r\n]*                      ;//-> channel(COMMENTS) ;
COMMENT_MEDIUM      : COMMENT_LONG_PREFIX  ~[\r\n]*? COMMENT_LONG_SUFFIX ;//-> channel(COMMENTS) ;
COMMENT_LONG        : COMMENT_LONG_PREFIX  .*?       COMMENT_LONG_SUFFIX ;//-> channel(COMMENTS) ;

COMMENT_SHORT_PREFIX : '//' ;
COMMENT_LONG_PREFIX : '/*' ;
COMMENT_LONG_SUFFIX : '*/' ;

IDENTIFIER            : (LETTER | DOLLAR | AND) (LETTER | NUMBER)* ;
fragment LETTER       : LETTER_UPPER | LETTER_LOWER ;
fragment LETTER_UPPER : 'A' .. 'Z' ;
fragment LETTER_LOWER : 'a' .. 'z' ;
fragment AND          : '&' ;
fragment DOLLAR       : '$' ;

OP_ASSIGN1  : '='  ; // sfall addition
OP_ASSIGN2  : ':=' ;
OP_INCREASE : '++' ;

NUMBER             : DIGIT+ ;
fragment DIGIT     : [0-9] ;

PAREN_OPEN  : '(' ;
PAREN_CLOSE : ')' ;
SEMICOLON   : ';' ;

EOL_DOS   : '\r\n' ;
EOL_UNIX  : '\n' ;
TAB       : '\t';
SPACE     : ' ' ;
