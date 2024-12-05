> from [Crafting Interpreters](http://www.craftinginterpreters.com/appendix-i.html)

> note: the `cpp` was just for syntax highlighting in vscode to make it look prettier than plain text

Syntax Grammar
```cpp
program        → declaration* EOF ;
```

Declaration Grammar
```cpp
declaration    -> varDecl 
                | statement 
                | funcDecl ;

varDecl        -> "var" IDENTIFIER ( "=" expression )? ";" ;
funcDecl   -> "fun" function ;  
```

Statement Grammar
```cpp
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
```cpp
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

miscellaneous
```cpp
arguments     -> expression ( "," expression )* ;
function      -> IDENTIFIER "(" parameters? ")" block ;
parameters    -> IDENTIFIER ( "," IDENTIFIER )* ;
```