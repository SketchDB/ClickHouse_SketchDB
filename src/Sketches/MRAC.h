#pragma once

#include <queue>

#include <Sketches/ISketch.h>

namespace DB
{

class MRAC : public ISketch
{
private:
    using sketch_t = uint32_t;
    unsigned int levels;
    unsigned int width;
    std::vector<std::vector<sketch_t>> sketch_counters;
    std::priority_queue<std::pair<flowkey_t, uint64_t>> flowkey_storage;

public:
    MRAC(unsigned int _levels, unsigned int /*_rows*/, unsigned int _width);

    virtual long getSize() const override { return levels * width; }

    virtual void update(flowkey_t flowkey) override;

    virtual unsigned int getNumberOfColumns(String query_name) const override;
    
    virtual DataTypePtr getColumnType(unsigned int column_idx) const override;

    virtual unsigned int getNumberOfRows(unsigned int column_idx) const override;

    virtual void insertIntoColumn(MutableColumnPtr & column, unsigned int column_idx, uint64_t sketch_total) const override;

private:
    uint32_t get_last_level(uint32_t level_hash);

    virtual void insertArgumentsIntoColumn(MutableColumnPtr & column, uint64_t sketch_total) const override;
    virtual void insertSketchCountersIntoColumn(MutableColumnPtr & column) const override;
    //std::vector<std::vector<sketch_t>> & getData() { return sketch_counters; }
};

}
