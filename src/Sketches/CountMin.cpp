#include <Sketches/CountMin.h>
#include <Sketches/SketchFactory.h>
#include <DataTypes/DataTypeFactory.h>

#include <algorithm>

#include <xxhash.h>

namespace DB
{

CountMin::CountMin(unsigned int /*_levels*/, unsigned int _rows, unsigned int _width)
    : ISketch("CountMin")
{
    rows = _rows;
    width = _width;
    sketch_counters.resize(rows);
    for(uint32_t i = 0; i < rows; i++)
    {
        sketch_counters[i].resize(width);
    }
    flowkey_storage_size = 100;
}

void CountMin::update(flowkey_t flowkey)
{
    Float64 estimate = updateCounters(flowkey);
    updateFlowkeyStorage(flowkey, estimate);
}

Float64 CountMin::updateCounters(flowkey_t flowkey)
{
    uint32_t sketch_column_index;
    std::vector<uint32_t> estimates(rows);

    for (uint32_t sketch_row_index = 0; sketch_row_index < rows; sketch_row_index++)
    {
        sketch_column_index = XXH32(static_cast<const void*>(&flowkey), sizeof(flowkey), sketch_row_index) % width;
        sketch_counters[sketch_row_index][sketch_column_index] += 1;
        estimates.push_back(sketch_counters[sketch_row_index][sketch_column_index]);
    }

    return static_cast<Float64>(*std::min_element(estimates.begin(), estimates.end()));
}

void CountMin::updateFlowkeyStorage(flowkey_t flowkey, Float64 estimate)
{
    if (flowkey_storage.size() < flowkey_storage_size)
    {
        flowkey_storage.emplace(flowkey, estimate);
    }
    else
    {
        auto it = std::find_if(flowkey_storage.begin(), flowkey_storage.end(), [flowkey](const std::pair<flowkey_t, uint32_t> & pair) {return pair.first == flowkey;});
        if (it != flowkey_storage.end())
        {
            flowkey_storage.erase(it);
            flowkey_storage.emplace(flowkey, estimate);
        }
        else
        {
            Float64 least_element = flowkey_storage.begin()->second;
            if (estimate < least_element)
            {
                return;
            }
            else
            {
                flowkey_storage.erase(flowkey_storage.begin());
                flowkey_storage.emplace(flowkey, estimate);
            }
        }
    }
}

void CountMin::insertSketchCountersIntoColumn(MutableColumnPtr & column) const
{
    for(uint32_t i = 0; i < rows; i++)
    {
        Array row_array(width);
        std::transform(
            sketch_counters[i].cbegin(), sketch_counters[i].cend(), row_array.begin(),
            [](sketch_t value) {return Field(value);}
        );
        column->insert(Field(row_array));
        //LOG_WARNING(log, "row_array size {}", row_array.size());
    }
}

unsigned int CountMin::getNumberOfColumns(String query_name) const
{
    if (query_name == "entropy")
    {
        return 1;
    }
    else if (query_name.find("heavy_hitter") != String::npos)
    {
        return 3;
    }
    assert(false);
    return 0;
}

DataTypePtr CountMin::getColumnType(unsigned int column_idx) const
{
    if (column_idx == 0)
    {
        return DataTypeFactory::instance().get("Array(Int32)");
    }
    else if (column_idx == 1)
    {
        return DataTypeFactory::instance().get("UInt64");
    }
    else if (column_idx == 2)
    {
        return DataTypeFactory::instance().get("Float64");
    }
    assert(false);
    return nullptr;
}

unsigned int CountMin::getNumberOfRows(unsigned int column_idx) const
{
    if (column_idx == 0)
    {
        return rows + 1;
    }
    else if (column_idx == 1 || column_idx == 2)
    {
        return flowkey_storage_size;
    }
    assert(false);
    return 0;
}

void CountMin::insertIntoColumn(MutableColumnPtr & column, unsigned int column_idx, uint64_t sketch_total) const
{
    if (column_idx == 0)
    {
        insertArgumentsIntoColumn(column, sketch_total);
        insertSketchCountersIntoColumn(column);
    }
    else if (column_idx == 1)
    {
        insertFlowkeysIntoColumn(column);
    }
    else if (column_idx == 2)
    {
        insertFlowkeyEstimatesIntoColumn(column);
    }
    else
    {
        assert(false);
    }
}

void CountMin::insertArgumentsIntoColumn(MutableColumnPtr & column, uint64_t sketch_total) const
{
	Array sketch_arguments_array(5);
	sketch_arguments_array[0] = Field(sketch_total);
	sketch_arguments_array[1] = Field(1);
	sketch_arguments_array[2] = Field(rows);
	sketch_arguments_array[3] = Field(width);
	sketch_arguments_array[4] = Field(flowkey_storage_size);
	column->insert(Field(sketch_arguments_array));
}

void CountMin::insertFlowkeysIntoColumn(MutableColumnPtr & column) const
{
    for (auto it = flowkey_storage.begin(); it != flowkey_storage.end(); it++)
    {
        column->insert(Field(it->first));
    }
}

void CountMin::insertFlowkeyEstimatesIntoColumn(MutableColumnPtr & column) const
{
    for (auto it = flowkey_storage.begin(); it != flowkey_storage.end(); it++)
    {
        column->insert(Field(it->second));
    }
}

void registerCountMin(SketchFactory & factory)
{
    factory.registerSketch("cm", [](const SketchFactory::Arguments & args)
    {
        return std::make_shared<CountMin>(args.levels, args.rows, args.width);
    });
}

}
