# UTF8-CPP: UTF-8 with C++ in a Portable Way

Many C++ developers miss an easy and portable way of handling Unicode encoded strings. The original C++ Standard (known as C++98 or C++03) is Unicode agnostic. C++11 provides some support for Unicode on core language and library level: u8, u, and U character and string literals, char16_t and char32_t character types, u16string and u32string library classes, and codecvt support for conversions between Unicode encoding forms. In the meantime, developers use third party libraries like ICU, OS specific capabilities, or simply roll out their own solutions.

In order to easily handle UTF-8 encoded Unicode strings, I came up with a small, C++98 compatible generic library. For anybody used to work with STL algorithms and iterators, it should be easy and natural to use. The code is freely available for any purpose - check out the license at the beginning of the utf8.h file. The library has been used a lot in the past ten years both in commercial and open-source projects and is considered feature-complete now. If you run into bugs or performance issues, please let me know and I'll do my best to address them.

----------

Simple usage:

``` c++
std::string s = "example";
utf8::iterator i (s.begin(), s.begin(), s.end());

// check input string for invalid utf8 encoding
auto endIt = utf8::find_invalid(s.begin(), s.end());
if (endIt != str.end()) {
    std::cerr << "Invalid UTF-8 encoding detected." << std::endl;
}

// convert input string to Unicode 
std::vector<unsigned long> strUnicode;
utf8::utf8to32(s.begin(), endIt, std::back_inserter(strUnicode));
```

More info can be found in the [docs][DOC].

[DOC]: https://github.com/nemtrif/utfcpp
