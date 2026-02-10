#include "sybase/SybReader.hpp"
#include "sybase/SybRow.hpp"
#include "db/DBException.hpp"

#ifdef WITH_SYBASE
#include <sybfront.h>
#include <sybdb.h>
#endif

#ifdef WITH_SYBASE

SybReader::SybReader(DBPROCESS* dbproc)
    : _dbproc(dbproc) {
    (void)dbproc;
    throw DBException("Sybase support not compiled in");
}

SybReader::~SybReader() = default;

bool SybReader::next() {
    if (!_dbproc) {
        return false;
    }
    
    RETCODE ret = dbnextrow(_dbproc);
    if (ret == NO_MORE_ROWS) {
        return false;
    }
    if (ret == FAIL || ret == BUF_FULL) {
        throw DBException("SybReader::next: dbnextrow failed");
    }
    
    // ret should be REG_ROW or one of the compute row types
    _row = std::make_unique<SybRow>(_dbproc);
    return true;
}

IDBRow& SybReader::row() {
    if (!_row) throw DBException("SybReader::row: no row");
    return *_row;
}

#endif