#pragma once

#include <AggregateFunctions/IAggregateFunction.h>
#include <AggregateFunctions/sketches/Utils.h>
#include <DataTypes/DataTypesNumber.h>
#include <Columns/ColumnVector.h>
#include <Columns/ColumnArray.h>
#include <Common/assert_cast.h>

#include <cmath>

namespace DB
{

struct HeavyHitterFlowkeysCountSketchData
{
    using sketch_t = int32_t;
    using flowkey_t = uint64_t;
    
    std::vector<std::vector<sketch_t>> sketch_array;
    std::vector<std::pair<flowkey_t, Float64>> flowkey_storage;
    uint64_t sketch_total;
    unsigned int levels, rows, width;
    unsigned int flowkey_storage_size;

    void add(const IColumn ** columns, size_t row_num, Poco::Logger * /*log*/)
    {
        // first column
        const auto & column1 = assert_cast<const ColumnArray &>(*columns[0]);
        Field row_field;
        column1.get(row_num, row_field);

        if (row_num == 0)
        {
            Array & arguments = row_field.get<const Array &>();
            sketch_total = arguments[0].get<const uint64_t &>();
            levels = static_cast<const unsigned int &>(arguments[1].get<const uint64_t &>());
            rows = static_cast<const unsigned int &>(arguments[2].get<const uint64_t &>());
            width = static_cast<const unsigned int &>(arguments[3].get<const uint64_t &>());
            flowkey_storage_size = static_cast<const unsigned int &>(arguments[4].get<const uint64_t &>());
            sketch_array.resize(rows);
            for(uint32_t i = 0; i < rows; i++)
            {
                sketch_array[i].resize(width);
            }
            //LOG_WARNING(log, "Count sketch arguments {} {} {}", sketch_total, rows, width);
        }
        else
        {
            if (row_num - 1 < rows)
            {
                for(uint32_t i = 0; i < width; i++)
                {
                    // TODO: Why is column an Array(long)?
                    Field & column_field = row_field.get<const Array &>()[i];
                    sketch_array[row_num - 1][i] = static_cast<const sketch_t &>(column_field.get<const long &>());
                }
            }
        }
        
        // process second and third column
        const auto & column2 = assert_cast<const ColumnVector<UInt64> &>(*columns[1]);
        const auto & column3 = assert_cast<const ColumnVector<Float64> &>(*columns[2]);
        Field row_field2, row_field3;
        column2.get(row_num, row_field2);
        column3.get(row_num, row_field3);
        const flowkey_t flowkey = row_field2.get<const flowkey_t &>();
        const Float64 flowkey_estimate = row_field3.get<const Float64 &>();

        flowkey_storage.push_back(std::make_pair(flowkey, flowkey_estimate));
    }

    Float64 get(Poco::Logger * /*log*/, unsigned int row_num) const
    {
        return flowkey_storage[row_num].first;
    }
};

class AggregateFunctionHeavyHitterFlowkeysCountSketch final : public IAggregateFunctionDataHelper<HeavyHitterFlowkeysCountSketchData, AggregateFunctionHeavyHitterFlowkeysCountSketch>
{
private:
    size_t num_args;
    Poco::Logger * log;

public:
    explicit AggregateFunctionHeavyHitterFlowkeysCountSketch(const DataTypes & argument_types_)
        : IAggregateFunctionDataHelper<HeavyHitterFlowkeysCountSketchData, AggregateFunctionHeavyHitterFlowkeysCountSketch>(argument_types_, {}, createResultType())
        , num_args(argument_types_.size())
        , log(&Poco::Logger::get("AggregateFunctionHeavyHitterFlowkeysCountSketch"))
    {
        LOG_WARNING(log, "num_args {}", num_args);
        for(size_t i = 0; i < num_args; i++)
            LOG_WARNING(log, "arg {} type {}", i, argument_types_[i]->getName());
    }

    String getName() const override
    {
        return "AggregateFunctionHeavyHitterFlowkeysCountSketch_sketch";
    }

    static DataTypePtr createResultType()
    {
        return std::make_shared<DataTypeNumber<Float64>>();
    }
    
    bool allocatesMemoryInArena() const override { return false; }

    void add(AggregateDataPtr __restrict place, const IColumn ** columns, size_t row_num, Arena *) const override
    {
        this->data(place).add(columns, row_num, log);
    }
    
    void merge(AggregateDataPtr __restrict /*place*/, ConstAggregateDataPtr /*rhs*/, Arena *) const override
    {
        //this->data(place).merge(this->data(rhs));
    }

    void serialize(ConstAggregateDataPtr __restrict /*place*/, WriteBuffer & /*buf*/, std::optional<size_t> /* version */) const override
    {
        //this->data(const_cast<AggregateDataPtr>(place)).serialize(buf);
    }

    void deserialize(AggregateDataPtr __restrict /*place*/, ReadBuffer & /*buf*/, std::optional<size_t> /* version */, Arena * /* arena */) const override
    {
        //this->data(place).deserialize(buf);
    }

    void insertResultInto(AggregateDataPtr __restrict place, IColumn & to, Arena *) const override
    {
        // TODO: column data type
        auto & column = assert_cast<ColumnVector<Float64> &>(to);
        for(unsigned int i = 0; i < this->data(place).flowkey_storage_size; i++)
        {
            column.getData().push_back(this->data(place).get(log, i));
        }
    }
};

}
