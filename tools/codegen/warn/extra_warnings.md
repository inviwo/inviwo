Name                                  | Clang        | GCC              | MSVC  |
--------------------------------------|--------------|------------------|-------|
value-to-bool                         | *no*         | *no*             | C4800 |
mismatched-tags                       | *same*       | *no*             | C4099 |
size_t-conversion                     | *no*         | *no*             | C4267 |
uninitialized                         | *same*       | *same*           | C4701 |
member-uninit                         | *no*         | *no*             | C26495 |
use-constexpr-for-functioncall        | *no*         | *no*             | C26498 |
arithmetic-overflow                   | *no*         | *no*             | C26451 |
no-unnamed-raii-objects               | *no*         | *no*             | C26444 |
special-noexcept                      | *no*         | *no*             | C26439 |