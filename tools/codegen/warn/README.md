# Usage
* To generate warning files, run from this folder
```python3 warn/warn.py --header header.h --extra_warnings extra_warnings.md --output_dir ../../../ext/warn/include```

* Add any extra warning to `extra_warnings.md` using the same format as in the default 
warning table in `warn/warnings.md` and re-run the script above

## References
- https://clang.llvm.org/docs/DiagnosticsReference.html
- https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html