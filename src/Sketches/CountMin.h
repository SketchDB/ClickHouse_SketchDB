#pragma once

#include <queue>

#include <Sketches/ISketch.h>

namespace DB
{

class CountMin : public ISketch
{
private:
    using sketch_t = uint32_t;
    unsigned int rows;
    unsigned int width;
    std::vector<std::vector<sketch_t>> sketch_counters;
	// flowkey storage
    unsigned int flowkey_storage_size;
    // define comparator for flowkey_storage that compares the second element of the pair
    struct CompareSecond
    {
        bool operator()(const std::pair<flowkey_t, Float64>& left, const std::pair<flowkey_t, Float64>& right) const
        {
            return left.second > right.second;
        }
    };
    std::set<std::pair<flowkey_t, Float64>, CompareSecond> flowkey_storage;

public:
    CountMin(unsigned int /*_levels*/, unsigned int _rows, unsigned int _width);

    virtual long getSize() const override { return rows * width; }

    virtual void update(flowkey_t flowkey) override;

    Float64 updateCounters(flowkey_t flowkey);

    void updateFlowkeyStorage(flowkey_t flowkey, Float64 estimate);
    
    virtual unsigned int getNumberOfColumns(String query_name) const override;
    
    virtual DataTypePtr getColumnType(unsigned int column_idx) const override;

    virtual unsigned int getNumberOfRows(unsigned int column_idx) const override;

    virtual void insertIntoColumn(MutableColumnPtr & column, unsigned int column_idx, uint64_t sketch_total) const override;
   
private:
    virtual void insertArgumentsIntoColumn(MutableColumnPtr & column, uint64_t sketch_total) const override;
    virtual void insertSketchCountersIntoColumn(MutableColumnPtr & column) const override;
    virtual void insertFlowkeysIntoColumn(MutableColumnPtr & column) const;
    virtual void insertFlowkeyEstimatesIntoColumn(MutableColumnPtr & column) const;
};

}
