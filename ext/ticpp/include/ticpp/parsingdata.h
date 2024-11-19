#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>
#include <ticpp/base.h>

#include <cassert>

class TiXmlParsingData {
    friend class TiXmlDocument;

public:
    void Stamp(const char* now);

    const TiXmlCursor& Cursor() { return cursor; }

private:
    // Only used by the document!
    TiXmlParsingData(const char* start, int _tabsize, int row, int col)
        : cursor{.row = row, .col = col}
        , stamp{TiXmlBase::SkipByteOrderMark(start)}
        , tabsize{_tabsize} {
        assert(start);
    }

    TiXmlCursor cursor;
    const char* stamp;
    int tabsize;
};
