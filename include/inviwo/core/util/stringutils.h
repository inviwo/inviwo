#ifndef IVW_STRINGUTILS_H
#define IVW_STRINGUTILS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <vector>
#include <string>

namespace inviwo {

    namespace strutil {

        // split at given delimiter
        IVW_CORE_API std::vector<std::string> split(const std::string &s,
                                               const std::string &delimiter = " ",
                                       size_t prereserved_size = 16);

        // trim from start (in place)
        IVW_CORE_API void ltrim(std::string &s);

        // trim from end (in place)
        IVW_CORE_API void rtrim(std::string &s);

        // trim from both ends (in place)
        IVW_CORE_API void trim(std::string &s);

        // trim from start (copying)
        IVW_CORE_API std::string ltrim_copy(std::string s);

        // trim from end (copying)
        IVW_CORE_API std::string rtrim_copy(std::string s);

        // trim from both ends (copying)
        IVW_CORE_API std::string trim_copy(std::string s);

    }  // namespace strutil

}  // namespace inviwo

#endif // IVW_STRINGUTILS_H
