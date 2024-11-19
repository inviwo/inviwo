#include <ticpp/parsingdata.h>

void TiXmlParsingData::Stamp(const char* now) {
    assert(now);

    // Do nothing if the tabsize is 0.
    if (tabsize < 1) {
        return;
    }

    // Get the current row, column.
    int row = cursor.row;
    int col = cursor.col;
    const char* p = stamp;
    assert(p);

    while (p < now) {
        // Treat p as unsigned, so we have a happy compiler.
        const unsigned char* pU = (const unsigned char*)p;

        // Code contributed by Fletcher Dunn: (modified by lee)
        switch (*pU) {
            case 0:
                // We *should* never get here, but in case we do, don't
                // advance past the terminating null character, ever
                return;

            case '\r':
                // bump down to the next line
                ++row;
                col = 0;
                // Eat the character
                ++p;

                // Check for \r\n sequence, and treat this as a single character
                if (*p == '\n') {
                    ++p;
                }
                break;

            case '\n':
                // bump down to the next line
                ++row;
                col = 0;

                // Eat the character
                ++p;

                // Check for \n\r sequence, and treat this as a single
                // character.  (Yes, this bizarre thing does occur still
                // on some arcane platforms...)
                if (*p == '\r') {
                    ++p;
                }
                break;

            case '\t':
                // Eat the character
                ++p;

                // Skip to next tab stop
                col = (col / tabsize + 1) * tabsize;
                break;

            default:
                // Eat the 1 to 4 byte utf8 character.
                int step = TiXmlBase::utf8ByteTable[*((const unsigned char*)p)];
                if (step == 0) step = 1;  // Error case from bad encoding, but handle gracefully.
                p += step;

                // Just advance one column, of course.
                ++col;
                break;
        }
    }
    cursor.row = row;
    cursor.col = col;
    assert(cursor.row >= -1);
    assert(cursor.col >= -1);
    stamp = p;
    assert(stamp);
}
