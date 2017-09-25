Name                             | Clang              | GCC                  | VS           
-------------------------------- | ------------------ | -------------------- | -------------
assign-base-inaccessible         | *no*               | *no*                 | C4626 (13)   
assign-could-not-be-generated    | *no*               | *no*                 | C4512 (13)   
automatic-inline                 | *no*               | *no*                 | C4711 (13)   
behavior-change                  | *no*               | *no*                 | C4350 (14)   
bool-conversion                  | *same* (3.2)       | *no*                 | *no*         
c++11-extensions                 | *same* (3.2)       | *no*                 | *no*         
c++98-compat                     | *same* (3.2)       | *no*                 | *no*         
c++98-compat-pedantic            | *same* (3.2)       | *no*                 | *no*         
cast-align                       | *same* (3.2)       | *same* (3.4)         | *no*         
catch-semantic-changed           | *no*               | *no*                 | C4571 (14)   
conditional-uninitialized        | *same* (3.2)       | *no*                 | *no*         
constant-conditional             | *no*               | *no*                 | C4127 (13)   
constant-conversion              | *same* (3.2)       | *no*                 | *no*         
conversion                       | *same* (3.2)       | *same* (3.4)         | C4244 (13)   
conversion-loss                  | conversion (3.2)   | conversion (3.4)     | C4242 (13)   
conversion-sign-extended         | *no*               | *no*                 | C4826 (14)   
copy-ctor-could-not-be-generated | *no*               | *no*                 | C4625 (13)   
covered-switch-default           | *same* (3.2)       | *no*                 | *no*         
deprecated                       | *same* (3.2)       | *same* (3.4)         | *no*         
deprecated-declarations          | *same* (3.2)       | *same* (3.4)         | C4996 (13)   
deprecated-objc-isa-usage        | *same* (3.3)       | *no*                 | *no*         
deprecated-register              | *same* (3.4)       | *no*                 | *no*         
dflt-ctor-base-inaccessible      | *no*               | *no*                 | C4623 (13)   
dflt-ctor-could-not-be-generated | *no*               | *no*                 | C4510 (13)   
digraphs-not-supported           | *no*               | *no*                 | C4628 (13.10)
dll-interface                    | *no*               | *no*                 | C4251 (13)   
dll-interface-base               | *no*               | *no*                 | C4275 (13)   
documentation                    | *same* (3.2)       | *no*                 | *no*         
documentation-unknown-command    | *same* (3.3)       | *no*                 | *no*         
empty-body                       | *same* (3.2)       | *same* (4.3)         | *no*         
enum-conversion                  | *same* (3.2)       | *no*                 | *no*         
exit-time-destructors            | *same* (3.2)       | *no*                 | *no*         
extra-semi                       | *same* (3.2)       | *no*                 | *no*         
force-not-inlined                | *no*               | *no*                 | C4714 (13)   
format                           | *same* (3.2)       | *same* (3.4)         | *no*         
four-char-constants              | *same* (3.2)       | *no*                 | *no*         
global-constructors              | *same* (3.2)       | *no*                 | *no*         
hide-virtual-func                | *no*               | *no*                 | C4263 (13)   
ill-formed-comma-expr            | unused-value (3.2) | unused-value (3.4)   | C4548 (13.10)
implicit-fallthrough             | *same* (3.2)       | *no*                 | *no*         
inconsistent-missing-override    | *same* (3.6)       | suggest-override (5) | *no*         
inherits-via-dominance           | *no*               | *no*                 | C4250 (13)   
int-conversion                   | *same* (3.2)       | *no*                 | *no*         
invalid-offsetof                 | *same* (3.2)       | *same* (3.4)         | *no*         
is-defined-to-be                 | *no*               | *no*                 | C4574 (16)   
layout-changed                   | *no*               | *no*                 | C4371 (13)   
misleading-indentation           | *no*               | *same* (6)           | *no*         
missing-braces                   | *same* (3.2)       | *same* (3.4)         | *no*         
missing-field-initializers       | *same* (3.2)       | *same* (4.0)         | *no*         
missing-noreturn                 | *same* (3.3)       | *same* (3.4)         | *no*         
name-length-exceeded             | *no*               | *no*                 | C4503 (13)   
newline-eof                      | *same* (3.4)       | *no*                 | *no*         
no-such-warning                  | *no*               | *no*                 | C4619 (13)   
non-virtual-dtor                 | *same* (3.2)       | *same* (3.4)         | C4265 (13)   
not-inlined                      | *no*               | *no*                 | C4710 (13)   
object-layout-change             | *no*               | *no*                 | C4435 (17)   
old-style-cast                   | *same* (3.5)       | *same* (3.4)         | *no*         
overloaded-virtual               | *same* (3.2)       | *same* (3.4)         | *no*         
padded                           | *same* (3.2)       | *same* (3.4)         | C4820 (13)   
parentheses                      | *same* (3.2)       | *same* (3.4)         | *no*         
pedantic                         | *same* (3.2)       | *same* (4.8)         | *no*         
return-type                      | *same* (3.2)       | *same* (3.4)         | *no*         
shadow                           | *same* (3.2)       | *same* (3.4)         | *no*         
shorten-64-to-32                 | *same* (3.2)       | *no*                 | *no*         
sign-compare                     | *same* (3.2)       | *same* (3.4)         | C4389 (13)   
sign-conversion                  | *same* (3.2)       | *same* (4.3)         | C4365 (14)   
signed-unsigned-compare          | sign-compare (3.2) | sign-compare (3.4)   | C4388 (18)   
static-ctor-not-thread-safe      | *no*               | *no*                 | C4640 (13)   
switch                           | *same* (3.2)       | *same* (3.4)         | C4062 (13)   
switch-enum                      | *same* (3.2)       | *same* (3.4)         | C4061 (13)   
this-used-in-init                | *no*               | *no*                 | C4355 (13)   
undef                            | *no*               | *same* (3.4)         | C4668 (13)   
uninitialized                    | *same* (3.2)       | *same* (3.4)         | *no*         
unknown-pragmas                  | *same* (3.2)       | *same* (3.4)         | *no*         
unreachable-code                 | *same* (3.5)       | *same* (3.4)         | C4702 (13)   
unreachable-code-return          | *same* (3.5)       | *no*                 | *no*         
unreferenced-inline              | *no*               | *no*                 | C4514 (13)   
unsafe-conversion                | *no*               | *no*                 | C4191 (13)   
unused-but-set-variable          | *no*               | *same* (4.6)         | *no*         
unused-function                  | *same* (3.2)       | *same* (3.4)         | *no*         
unused-label                     | *same* (3.2)       | *same* (3.4)         | *no*         
unused-parameter                 | *same* (3.2)       | *same* (3.4)         | C4100 (13)   
unused-value                     | *same* (3.2)       | *same* (3.4)         | C4555 (13)   
unused-variable                  | *same* (3.2)       | *same* (3.4)         | C4189 (13)   
used-but-marked-unused           | *same* (3.2)       | *no*                 | *no*         
user-ctor-required               | *no*               | *no*                 | C4610 (13)   
