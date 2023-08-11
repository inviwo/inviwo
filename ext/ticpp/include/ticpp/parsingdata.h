#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>
#include <ticpp/base.h>

#include <cassert>

class TiXmlParsingData {
    friend class TiXmlDocument;

public:
    void Stamp(const char* now, TiXmlEncoding encoding);

    const TiXmlCursor& Cursor() { return cursor; }

private:
    // Only used by the document!
    TiXmlParsingData(const char* start, int _tabsize, int row, int col) {
        assert(start);
        stamp = start;
        tabsize = _tabsize;
        cursor.row = row;
        cursor.col = col;
    }

    TiXmlCursor cursor;
    const char* stamp;
    int tabsize;
};
