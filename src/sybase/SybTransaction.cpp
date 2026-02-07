#include "SybTransaction.hpp"
#include "db/DBException.hpp"

SybTransaction::SybTransaction() = default;
SybTransaction::~SybTransaction() = default;

void SybTransaction::commit() {
    if (!_active) throw DBException("SybTransaction::commit: not active");
    _active = false;
}

void SybTransaction::rollback() {
    if (!_active) throw DBException("SybTransaction::rollback: not active");
    _active = false;
}
