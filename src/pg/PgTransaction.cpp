#include "PgTransaction.hpp"
#include "db/DBException.hpp"

PgTransaction::PgTransaction() = default;
PgTransaction::~PgTransaction() = default;

void PgTransaction::commit() {
    if (!_active) throw DBException("PgTransaction::commit: not active");
    _active = false;
}

void PgTransaction::rollback() {
    if (!_active) throw DBException("PgTransaction::rollback: not active");
    _active = false;
}
