#include "SybTransaction.hpp"
#include "db/DBException.hpp"
#include "sybase/SybConnection.hpp"

#ifdef WITH_SYBASE
#include <sybfront.h>
#include <sybdb.h>
#endif

SybTransaction::SybTransaction(SybConnection* conn)
    : _conn(conn) {}

SybTransaction::~SybTransaction() {
    // Auto-rollback if still active
    if (_active && _conn) {
        try {
            rollback();
        } catch (...) {
            // Ignore errors in destructor
        }
    }
}

void SybTransaction::commit() {
#ifdef WITH_SYBASE
    if (!_active) throw DBException("SybTransaction::commit: not active");
    if (!_conn || !_conn->getDbProcess()) {
        throw DBException("SybTransaction::commit: Connection is null");
    }
    
    DBPROCESS* dbproc = _conn->getDbProcess();
    
    if (dbcmd(dbproc, "COMMIT TRAN") == FAIL) {
        throw DBException("SybTransaction::commit: dbcmd failed");
    }
    
    if (dbsqlexec(dbproc) == FAIL) {
        throw DBException("SybTransaction::commit: dbsqlexec failed");
    }
    
    // Consume results
    while (dbresults(dbproc) != NO_MORE_RESULTS) {
        // Empty loop to consume all results
    }
    
    _active = false;
#else
    throw DBException("Sybase support not compiled in");
#endif
}

void SybTransaction::rollback() {
#ifdef WITH_SYBASE
    if (!_active) throw DBException("SybTransaction::rollback: not active");
    if (!_conn || !_conn->getDbProcess()) {
        throw DBException("SybTransaction::rollback: Connection is null");
    }
    
    DBPROCESS* dbproc = _conn->getDbProcess();
    
    if (dbcmd(dbproc, "ROLLBACK TRAN") == FAIL) {
        throw DBException("SybTransaction::rollback: dbcmd failed");
    }
    
    if (dbsqlexec(dbproc) == FAIL) {
        throw DBException("SybTransaction::rollback: dbsqlexec failed");
    }
    
    // Consume results
    while (dbresults(dbproc) != NO_MORE_RESULTS) {
        // Empty loop to consume all results
    }
    
    _active = false;
#else
    throw DBException("Sybase support not compiled in");
#endif
}
