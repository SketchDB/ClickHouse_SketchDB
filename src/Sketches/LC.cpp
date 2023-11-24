#include <Sketches/LC.h>
#include <Sketches/SketchFactory.h>
#include <DataTypes/DataTypeFactory.h>

#include <xxhash.h>

namespace DB
{

LC::LC(unsigned int /*_levels*/, unsigned int /*_rows*/, unsigned int _width)
    : ISketch("LC")
{
    width = _width;
    sketch_counters.resize(width);
}

void LC::update(flowkey_t flowkey)
{
    const uint32_t hash_seed = 0;
    uint32_t sketch_column_index = XXH32(static_cast<const void*>(&flowkey), sizeof(flowkey), hash_seed) % width;
    sketch_counters[sketch_column_index] = 1;
}

void LC::insertSketchCountersIntoColumn(MutableColumnPtr & column) const
{
    Array level_array(width);
    std::transform(
        sketch_counters.cbegin(), sketch_counters.cend(), level_array.begin(),
        [](sketch_t value) {return Field(value);}
    );
    column->insert(Field(level_array));
}

unsigned int LC::getNumberOfColumns(String query_name) const
{
    if (query_name == "cardinality")
    {
        return 1;
    }
    assert(false);
    return 0;
}

DataTypePtr LC::getColumnType(unsigned int column_idx) const
{
    if (column_idx == 0)
    {
        // TODO: this should go back to Array(Bool)
        //return DataTypeFactory::instance().get("Array(Bool)");
        return DataTypeFactory::instance().get("Array(UInt32)");
    }
    assert(false);
    return nullptr;
}

unsigned int LC::getNumberOfRows(unsigned int column_idx) const
{
    if (column_idx == 0)
    {
        return 1;
    }
    assert(false);
    return 0;
}

void LC::insertIntoColumn(MutableColumnPtr & column, unsigned int column_idx, uint64_t sketch_total) const
{
    if (column_idx == 0)
    {
        insertArgumentsIntoColumn(column, sketch_total);
        insertSketchCountersIntoColumn(column);
    }
}

void LC::insertArgumentsIntoColumn(MutableColumnPtr & column, uint64_t sketch_total) const
{
	Array sketch_arguments_array(4);
	sketch_arguments_array[0] = Field(sketch_total);
	sketch_arguments_array[1] = Field(1);
	sketch_arguments_array[2] = Field(1);
	sketch_arguments_array[3] = Field(width);
	column->insert(Field(sketch_arguments_array));
}

void registerLC(SketchFactory & factory)
{
    factory.registerSketch("lc", [](const SketchFactory::Arguments & args)
    {
        return std::make_shared<LC>(args.levels, args.rows, args.width);
    });
}

}
