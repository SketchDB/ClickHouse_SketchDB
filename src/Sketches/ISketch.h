#pragma once

#include <base/types.h>
#include <Columns/IColumn.h>
#include <DataTypes/DataTypeArray.h>

#include <cstdint>
#include <memory>

namespace DB
{

using flowkey_t = uint64_t;

namespace ErrorCodes
{
    extern const int NOT_IMPLEMENTED;
}

class ISketch : public std::enable_shared_from_this<ISketch>
{
private:
    String name;

protected:
    Poco::Logger * log;

public:
    ISketch(String);

    String getName() { return name; }

    virtual long getSize() const = 0;

    virtual void update(flowkey_t flowkey) = 0;

    virtual unsigned int getNumberOfColumns(String query_name) const = 0;

    virtual DataTypePtr getColumnType(unsigned int column_idx) const = 0;

    virtual unsigned int getNumberOfRows(unsigned int column_idx) const = 0;

    virtual void insertIntoColumn(MutableColumnPtr & column, unsigned int column_idx, uint64_t sketch_total) const = 0;

    virtual ~ISketch() = default;

private:
    virtual void insertArgumentsIntoColumn(MutableColumnPtr & column, uint64_t sketch_total) const = 0;
    virtual void insertSketchCountersIntoColumn(MutableColumnPtr & column) const = 0;
};

using SketchPtr = std::shared_ptr<ISketch>;

}
