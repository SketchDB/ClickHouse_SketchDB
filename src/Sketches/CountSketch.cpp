#include <Sketches/CountSketch.h>
#include <Sketches/SketchFactory.h>
#include <DataTypes/DataTypeFactory.h>

#include <algorithm>

#include <xxhash.h>

namespace DB
{

Float64 median(std::vector<int32_t> values)
{
    const size_t & n = values.size();
    std::sort(values.begin(), values.end());
    if (n % 2 == 0)
    {
        return static_cast<Float64>(values[(n-2) / 2] + values[n / 2]) / 2.0;
    }
    else
    {
        return static_cast<Float64>(values[(n-1) / 2]);
    }
}

CountSketch::CountSketch(unsigned int /*_levels*/, unsigned int _rows, unsigned int _width)
    : ISketch("CountSketch")
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

void CountSketch::update(flowkey_t flowkey)
{
    Float64 estimate = updateCounters(flowkey);
    updateFlowkeyStorage(flowkey, estimate);
}

Float64 CountSketch::updateCounters(flowkey_t flowkey)
{
    int32_t increment;
    uint32_t sketch_column_index;
    std::vector<int32_t> estimates(rows);

    for (uint32_t sketch_row_index = 0; sketch_row_index < rows; sketch_row_index++)
    {
        increment = XXH32(static_cast<const void*>(&flowkey), sizeof(flowkey), sketch_row_index + rows) % 2;
        increment = increment * 2 - 1;
        sketch_column_index = XXH32(static_cast<const void*>(&flowkey), sizeof(flowkey), sketch_row_index) % width;
        sketch_counters[sketch_row_index][sketch_column_index] += increment;
        //estimates.push_back(sketch_counters[sketch_row_index][sketch_column_index] * increment);
        estimates[sketch_row_index] = sketch_counters[sketch_row_index][sketch_column_index] * increment;
    }

    return median(estimates);
}

void CountSketch::updateFlowkeyStorage(flowkey_t flowkey, Float64 estimate)
{
    auto it = std::find_if(flowkey_storage.begin(), flowkey_storage.end(), [flowkey](const std::pair<flowkey_t, uint32_t> & pair) {return pair.first == flowkey;});
    if (it != flowkey_storage.end())
    {
        flowkey_storage.erase(it);
    }
    else
    {
        if (flowkey_storage.size() < flowkey_storage_size)
        {
        }
        else
        {
            //Float64 least_element = flowkey_storage.begin()->second;
            Float64 least_element = std::prev(flowkey_storage.end())->second;
            if (estimate < least_element)
            {
                return;
            }
            else
            {
                //flowkey_storage.erase(flowkey_storage.begin());
                flowkey_storage.erase(std::prev(flowkey_storage.end()));
            }
        }
    }
    flowkey_storage.emplace(flowkey, estimate);
}

void CountSketch::insertSketchCountersIntoColumn(MutableColumnPtr & column) const
{
    for(uint32_t i = 0; i < rows; i++)
    {
        Array row_array(width);
        std::transform(
            sketch_counters[i].cbegin(), sketch_counters[i].cend(), row_array.begin(),
            [](sketch_t value) {return Field(value);}
        );
        column->insert(Field(row_array));
    }
}

unsigned int CountSketch::getNumberOfColumns(String query_name) const
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

DataTypePtr CountSketch::getColumnType(unsigned int column_idx) const
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

unsigned int CountSketch::getNumberOfRows(unsigned int column_idx) const
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

void CountSketch::insertIntoColumn(MutableColumnPtr & column, unsigned int column_idx, uint64_t sketch_total) const
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

void CountSketch::insertArgumentsIntoColumn(MutableColumnPtr & column, uint64_t sketch_total) const
{
	Array sketch_arguments_array(5);
	sketch_arguments_array[0] = Field(sketch_total);
	sketch_arguments_array[1] = Field(1);
	sketch_arguments_array[2] = Field(rows);
	sketch_arguments_array[3] = Field(width);
	sketch_arguments_array[4] = Field(flowkey_storage_size);
	column->insert(Field(sketch_arguments_array));
}

void CountSketch::insertFlowkeysIntoColumn(MutableColumnPtr & column) const
{
    for (auto it = flowkey_storage.begin(); it != flowkey_storage.end(); it++)
    {
        column->insert(Field(it->first));
    }
}

void CountSketch::insertFlowkeyEstimatesIntoColumn(MutableColumnPtr & column) const
{
    for (auto it = flowkey_storage.begin(); it != flowkey_storage.end(); it++)
    {
        column->insert(Field(it->second));
    }
}

void registerCountSketch(SketchFactory & factory)
{
    factory.registerSketch("cs", [](const SketchFactory::Arguments & args)
    {
        return std::make_shared<CountSketch>(args.levels, args.rows, args.width);
    });
}

}
