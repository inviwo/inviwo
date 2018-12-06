#include <inviwo/core/util/stringutils.h>

#include <algorithm>
#include <cctype>

namespace inviwo {

    namespace strutil {

        // split at given delimiter
        std::vector<std::string> split(const std::string &s, const std::string &delimiter, size_t prereserved_size) {
            std::vector<std::string> elements;
            elements.reserve(prereserved_size);

            size_t last = 0;
            size_t next = 0;
            while ((next = s.find(delimiter, last)) != std::string::npos) {
                elements.push_back(s.substr(last, next - last));
                last = next + 1;
            }
            elements.push_back(s.substr(last));

            return elements;
        }

        // trim from start (in place)
        void ltrim(std::string &s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
        }

        // trim from end (in place)
        void rtrim(std::string &s) {
            s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(),
                    s.end());
        }

        // trim from both ends (in place)
        void trim(std::string &s) {
            ltrim(s);
            rtrim(s);
        }

        // trim from start (copying)
        std::string ltrim_copy(std::string s) {
            ltrim(s);
            return s;
        }

        // trim from end (copying)
        std::string rtrim_copy(std::string s) {
            rtrim(s);
            return s;
        }

        // trim from both ends (copying)
        std::string trim_copy(std::string s) {
            trim(s);
            return s;
        }

    } // namespace struril

} // namespace inviwo
