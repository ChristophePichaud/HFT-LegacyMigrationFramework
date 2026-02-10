#include "sybase/SybRow.hpp"
#include "sybase/SybValue.hpp"
#include "db/DBException.hpp"

#ifdef WITH_SYBASE
#include <sybfront.h>
#include <sybdb.h>
#endif

#ifdef WITH_SYBASE
SybRow::SybRow(DBPROCESS* dbproc) {

    if (!dbproc) {
        throw DBException("SybRow: dbproc is null");
    }
    
    int numCols = dbnumcols(dbproc);
    _values.reserve(numCols);
    
    for (int col = 1; col <= numCols; ++col) {  // Sybase columns are 1-based
        BYTE* data = dbdata(dbproc, col);
        DBINT datalen = dbdatlen(dbproc, col);
        
        bool isNull = (data == nullptr || datalen == 0);
        std::string value;
        
        if (!isNull) {
            int coltype = dbcoltype(dbproc, col);
            
            // Convert data based on type
            switch (coltype) {
                case SYBINT1:
                case SYBINT2:
                case SYBINT4:
                case SYBINT8: {
                    DBINT intval = 0;
                    dbconvert(dbproc, coltype, data, datalen, SYBINT4, (BYTE*)&intval, -1);
                    value = std::to_string(intval);
                    break;
                }
                case SYBREAL:
                case SYBFLT8: {
                    DBFLT8 dblval = 0.0;
                    dbconvert(dbproc, coltype, data, datalen, SYBFLT8, (BYTE*)&dblval, -1);
                    value = std::to_string(dblval);
                    break;
                }
                case SYBCHAR:
                case SYBVARCHAR:
                case SYBTEXT:
                    value.assign((char*)data, datalen);
                    break;
                default:
                    // For other types, try to convert to string
                    char buffer[256];
                    DBINT converted = dbconvert(dbproc, coltype, data, datalen, 
                                               SYBCHAR, (BYTE*)buffer, sizeof(buffer));
                    if (converted >= 0) {
                        value.assign(buffer, converted);
                    }
                    break;
            }
        }
        
        _values.push_back(std::make_unique<SybValue>(value, isNull));
    }
}

SybRow::~SybRow() = default;

std::size_t SybRow::columnCount() const {
    return _values.size();
}

const IDBValue& SybRow::operator[](std::size_t idx) const {
    if (idx >= _values.size()) {
        throw DBException("SybRow::operator[]: index out of range");
    }
    return *_values[idx];
}

#endif