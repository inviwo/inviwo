Name                              | Clang              | GCC                  | VS                
--------------------------------- | ------------------ | -------------------- | ------------------
arithmetic-overflow               | *no*               | *no*                 | C26451 (15)       
assign-base-inaccessible          | *no*               | *no*                 | C4626 (12)        
assign-could-not-be-generated     | *no*               | *no*                 | C4512 (13)        
automatic-inline                  | *no*               | *no*                 | C4711 (12)        
behavior-change                   | *no*               | *no*                 | C4350 (13)        
bool-conversion                   | *same* (3.2)       | *no*                 | *no*              
c++11-extensions                  | *same* (3.2)       | *no*                 | *no*              
c++98-compat                      | *same* (3.2)       | *no*                 | *no*              
c++98-compat-pedantic             | *same* (3.2)       | *no*                 | *no*              
cast-align                        | *same* (3.2)       | *same* (3.4)         | *no*              
catch-semantic-changed            | *no*               | *no*                 | C4571 (13)        
char-subscripts                   | *same* (3.2)       | *same* (3.4)         | *no*              
class-memaccess                   | *no*               | *same* (8)           | *no*              
conditional-uninitialized         | *same* (3.2)       | *no*                 | *no*              
constant-conditional              | *no*               | *no*                 | C4127 (13)        
constant-conversion               | *same* (3.2)       | *no*                 | *no*              
conversion                        | *same* (3.2)       | *same* (3.4)         | C4244 (13)        
conversion-loss                   | conversion (3.2)   | conversion (3.4)     | C4242 (13)        
conversion-sign-extended          | *no*               | *no*                 | C4826 (13)        
copy-ctor-could-not-be-generated  | *no*               | *no*                 | C4625 (12)        
covered-switch-default            | *same* (3.2)       | *no*                 | *no*              
deprecated                        | *same* (3.2)       | *same* (3.4)         | *no*              
deprecated-declarations           | *same* (3.2)       | *same* (3.4)         | C4996 (12)        
deprecated-objc-isa-usage         | *same* (3.3)       | *no*                 | *no*              
deprecated-register               | *same* (3.4)       | *no*                 | *no*              
dflt-ctor-base-inaccessible       | *no*               | *no*                 | C4623 (12)        
dflt-ctor-could-not-be-generated  | *no*               | *no*                 | C4510 (13)        
digraphs-not-supported            | *no*               | *no*                 | C4628 (13.00.9466)
dll-interface                     | *no*               | *no*                 | C4251 (13)        
dll-interface-base                | *no*               | *no*                 | C4275 (13)        
documentation                     | *same* (3.2)       | *no*                 | *no*              
documentation-unknown-command     | *same* (3.3)       | *no*                 | *no*              
empty-body                        | *same* (3.2)       | *same* (4.3)         | *no*              
enum-conversion                   | *same* (3.2)       | *no*                 | *no*              
exit-time-destructors             | *same* (3.2)       | *no*                 | *no*              
extra-semi                        | *same* (3.2)       | *no*                 | *no*              
force-not-inlined                 | *no*               | *no*                 | C4714 (12)        
format                            | *same* (3.2)       | *same* (3.4)         | *no*              
four-char-constants               | *same* (3.2)       | *no*                 | *no*              
global-constructors               | *same* (3.2)       | *no*                 | *no*              
gnu-zero-variadic-macro-arguments | *same* (3.4)       | *no*                 | *no*              
hide-virtual-func                 | *no*               | *no*                 | C4263 (12)        
ignored-attributes                | *same* (3.2)       | *same* (6)           | *no*              
ill-formed-comma-expr             | unused-value (3.2) | unused-value (3.4)   | C4548 (13.00.9466)
implicit-fallthrough              | *same* (3.2)       | *no*                 | *no*              
inconsistent-missing-override     | *same* (3.6)       | suggest-override (5) | *no*              
inherits-via-dominance            | *no*               | *no*                 | C4250 (13)        
int-conversion                    | *same* (3.2)       | *no*                 | *no*              
int-in-bool-context               | *no*               | *same* (7)           | *no*              
invalid-offsetof                  | *same* (3.2)       | *same* (3.4)         | *no*              
is-defined-to-be                  | *no*               | *no*                 | C4574 (15)        
layout-changed                    | *no*               | *no*                 | C4371 (12)        
member-uninit                     | *no*               | *no*                 | C26495 (15)       
misleading-indentation            | *no*               | *same* (6)           | *no*              
mismatched-tags                   | *same* (3.2)       | *no*                 | C4099 (13)        
missing-braces                    | *same* (3.2)       | *same* (3.4)         | *no*              
missing-field-initializers        | *same* (3.2)       | *same* (4.0)         | *no*              
missing-noreturn                  | *same* (3.3)       | *same* (3.4)         | *no*              
name-length-exceeded              | *no*               | *no*                 | C4503 (13)        
namespace-uses-itself             | *no*               | *no*                 | C4515 (13)        
newline-eof                       | *same* (3.4)       | *no*                 | *no*              
no-such-warning                   | *no*               | *no*                 | C4619 (12)        
no-unnamed-raii-objects           | *no*               | *no*                 | C26444 (15)       
non-virtual-dtor                  | *same* (3.2)       | *same* (3.4)         | C4265 (12)        
not-inlined                       | *no*               | *no*                 | C4710 (13)        
null-pointer-arithmetic           | *same* (6)         | *no*                 | *no*              
object-layout-change              | *no*               | *no*                 | C4435 (16)        
old-style-cast                    | *same* (3.5)       | *same* (3.4)         | *no*              
overloaded-virtual                | *same* (3.2)       | *same* (3.4)         | *no*              
padded                            | *same* (3.2)       | *same* (3.4)         | C4820 (12)        
parentheses                       | *same* (3.2)       | *same* (3.4)         | *no*              
pedantic                          | *same* (3.2)       | *same* (4.8)         | *no*              
restrict                          | *no*               | *same* (7)           | *no*              
return-type                       | *same* (3.2)       | *same* (3.4)         | *no*              
self-assign-overloaded            | *same* (7)         | *no*                 | *no*              
shadow                            | *same* (3.2)       | *same* (3.4)         | *no*              
shorten-64-to-32                  | *same* (3.2)       | *no*                 | *no*              
sign-compare                      | *same* (3.2)       | *same* (3.4)         | C4389 (12)        
sign-conversion                   | *same* (3.2)       | *same* (4.3)         | C4365 (13)        
signed-unsigned-compare           | sign-compare (3.2) | sign-compare (3.4)   | C4388 (17)        
size_t-conversion                 | *no*               | *no*                 | C4267 (12)        
special-noexcept                  | *no*               | *no*                 | C26439 (15)       
static-ctor-not-thread-safe       | *no*               | *no*                 | C4640 (13)        
stringop-truncation               | *no*               | *same* (8)           | *no*              
switch                            | *same* (3.2)       | *same* (3.4)         | C4062 (13)        
switch-enum                       | *same* (3.2)       | *same* (3.4)         | C4061 (13)        
this-used-in-init                 | *no*               | *no*                 | C4355 (13)        
undef                             | *no*               | *same* (3.4)         | C4668 (12)        
uninitialized                     | *same* (3.2)       | *same* (3.4)         | C4701 (12)        
unknown-pragmas                   | *same* (3.2)       | *same* (3.4)         | *no*              
unreachable-code                  | *same* (3.5)       | *same* (3.4)         | C4702 (12)        
unreachable-code-return           | *same* (3.5)       | *no*                 | *no*              
unreferenced-inline               | *no*               | *no*                 | C4514 (13)        
unsafe-conversion                 | *no*               | *no*                 | C4191 (13)        
unused-but-set-variable           | *no*               | *same* (4.6)         | *no*              
unused-function                   | *same* (3.2)       | *same* (3.4)         | *no*              
unused-label                      | *same* (3.2)       | *same* (3.4)         | *no*              
unused-parameter                  | *same* (3.2)       | *same* (3.4)         | C4100 (13)        
unused-value                      | *same* (3.2)       | *same* (3.4)         | C4555 (12)        
unused-variable                   | *same* (3.2)       | *same* (3.4)         | C4189 (13)        
use-constexpr-for-functioncall    | *no*               | *no*                 | C26498 (15)       
used-but-marked-unused            | *same* (3.2)       | *no*                 | *no*              
user-ctor-required                | *no*               | *no*                 | C4610 (13)        
value-to-bool                     | *no*               | *no*                 | C4800 (13)        
