#pragma once

#include <queue>

#include <Sketches/ISketch.h>

namespace DB
{

class LC : public ISketch
{
private:
    //using sketch_t = bool;
    // TODO: this should go back to Array(Bool)
    using sketch_t = uint32_t;
    unsigned int width;
    std::vector<sketch_t> sketch_counters;

public:
    LC(unsigned int /*_levels*/, unsigned int /*_rows*/, unsigned int _width);

    virtual long getSize() const override { return width; }

    virtual void update(flowkey_t flowkey) override;

    virtual unsigned int getNumberOfColumns(String query_name) const override;
    
    virtual DataTypePtr getColumnType(unsigned int column_idx) const override;

    virtual unsigned int getNumberOfRows(unsigned int column_idx) const override;

    virtual void insertIntoColumn(MutableColumnPtr & column, unsigned int column_idx, uint64_t sketch_total) const override;

private:
    virtual void insertArgumentsIntoColumn(MutableColumnPtr & column, uint64_t sketch_total) const override;
    virtual void insertSketchCountersIntoColumn(MutableColumnPtr & column) const override;
};

}

