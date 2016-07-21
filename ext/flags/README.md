# Flags for C++ 11 enum classes

C++ 11 gives a new type of enums — *enum classes*.
These are strongly typed enumerations, they do not cast implicitly to or from
the underlying integer type. They also do not leak enumeration values to
enclosing scope (thus the names of this values do not clash and you no longer
need to prefix/suffix them with something to disambiguate).

The only drawback to type safety is you cannot treat variables of *enum class*
types as sets of flags. That is because *enum classes* do not cast to integers
and there are no bitwise operators overloads defined for them.

This library brings a `flags` class template which provides bit flags for
scoped enums.

----------

Simple usage:

``` c++
enum class MyEnum { Value1 = 1 << 0, Value2 = 1 << 1 };
ALLOW_FLAGS_FOR_ENUM(MyEnum)

int main() {
  auto mask1 = MyEnum::Value1 | MyEnum::Value2; // set flags Value1 and Value 2
  if (mask1 & MyEnum::Value2) { // if Value2 flag is set
    /* ... */
  }
}
```

More info can be found in the [docs][DOC].

[DOC]: http://grisumbras.github.io/enum-flags/
