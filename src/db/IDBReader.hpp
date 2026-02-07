#pragma once

class IDBRow;

class IDBReader {
public:
    virtual ~IDBReader() = default;
    virtual bool next() = 0;
    virtual IDBRow& row() = 0;
};
