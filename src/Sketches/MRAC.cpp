#include <Sketches/MRAC.h>
#include <Sketches/SketchFactory.h>
#include <DataTypes/DataTypeFactory.h>

#include <xxhash.h>

namespace DB
{

MRAC::MRAC(unsigned int _levels, unsigned int /*_rows*/, unsigned int _width)
    : ISketch("MRAC")
{
    levels = _levels;
    width = _width;
    sketch_counters.resize(levels);
    for(uint32_t i = 0; i < levels; i++)
    {
        sketch_counters[i].resize(width);
    }
}

void MRAC::update(flowkey_t flowkey)
{
    const uint32_t sketch_row_index = 0;
    const uint32_t level_hash_seed = 1;
    uint32_t sketch_column_index = XXH32(static_cast<const void*>(&flowkey), sizeof(flowkey), sketch_row_index) % width;
    uint32_t level_hash = XXH32(static_cast<const void*>(&flowkey), sizeof(flowkey), level_hash_seed) % (1 << levels);
    uint32_t level = get_last_level(level_hash);
    if (level < levels)
    {
        sketch_counters[level][sketch_column_index] += 1;
    }
}

uint32_t MRAC::get_last_level(uint32_t level_hash)
{
    uint32_t last_level = 0;
    for(int32_t i = levels - 1; i >= 0; i--)
    {
        if (((level_hash >> i) & 1) == 0)
        {
            return last_level;
        }
        last_level++;
    }
    return last_level;
}

void MRAC::insertSketchCountersIntoColumn(MutableColumnPtr & column) const
{
    for(uint32_t i = 0; i < levels; i++)
    {
        Array level_array(width);
        std::transform(
            sketch_counters[i].cbegin(), sketch_counters[i].cend(), level_array.begin(),
            [](sketch_t value) {return Field(value);}
        );
        column->insert(Field(level_array));
        //LOG_WARNING(log, "row_array size {}", row_array.size());
    }
}

unsigned int MRAC::getNumberOfColumns(String query_name) const
{
    if (query_name == "entropy")
    {
        return 1;
    }
    assert(false);
    return 0;
}

DataTypePtr MRAC::getColumnType(unsigned int column_idx) const
{
    if (column_idx == 0)
    {
        return DataTypeFactory::instance().get("Array(UInt32)");
    }
    assert(false);
    return nullptr;
}

unsigned int MRAC::getNumberOfRows(unsigned int column_idx) const
{
    if (column_idx == 0)
    {
        return levels + 1;
    }
    assert(false);
    return 0;
}

void MRAC::insertIntoColumn(MutableColumnPtr & column, unsigned int column_idx, uint64_t sketch_total) const
{
    if (column_idx == 0)
    {
        insertArgumentsIntoColumn(column, sketch_total);
        insertSketchCountersIntoColumn(column);
    }
}

void MRAC::insertArgumentsIntoColumn(MutableColumnPtr & column, uint64_t sketch_total) const
{
	Array sketch_arguments_array(4);
	sketch_arguments_array[0] = Field(sketch_total);
	sketch_arguments_array[1] = Field(levels);
	sketch_arguments_array[2] = Field(1);
	sketch_arguments_array[3] = Field(width);
	column->insert(Field(sketch_arguments_array));
}

void registerMRAC(SketchFactory & factory)
{
    factory.registerSketch("mrac", [](const SketchFactory::Arguments & args)
    {
        return std::make_shared<MRAC>(args.levels, args.rows, args.width);
    });
}

}
