> from [Crafting Interpreters](http://www.craftinginterpreters.com/appendix-i.html)

Syntax Grammar
```shell
program        → declaration* EOF ;
```

Declaration Grammar
```shell
declaration    -> varDecl | statement ;

varDecl        -> "var" IDENTIFIER ( "=" expression )? ";" ;
```

Statement Grammar
```shell
statement      -> exprStmt 
                | printStmt 
                | block 
                | ifStmt
                | whileStmt
                | forStmt ;

exprStmt       -> expression ";" ;
printStmt      -> "print" expression ";" ;
block          -> "{" declaration* "}" ;
ifStmt         -> "if" "(" expression ")" statement ( "else" statement )? ;
whileStmt      -> "while" "(" expression ")" statement ;
forStmt        -> "for" "(" ( varDecl | exprStmt | ";" )
                           expression? ";"
                           expression? ")" statement ;
```

Expression Grammar
```shell
expression     -> assignment ;

assignment     -> IDENTIFIER "=" assignment
               | logic_or ;

logic_or       -> logic_and ( "or" logic_and )* ;
logic_and      -> equality ( "and" equality )* ;
equality       -> comparison ( ( "!=" | "==" ) comparison )* ;
comparison     -> term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           -> factor ( ( "-" | "+" ) factor )* ;
factor         -> unary ( ( "/" | "*" ) unary )* ;
unary          -> ( "!" | "-" ) unary
                | call ;
call           -> primary ( "(" arguments? ")" )* ;
primary        -> NUMBER | STRING | "true" | "false" | "nil"
               | "(" expression ")" | IDENTIFIER ;
```
