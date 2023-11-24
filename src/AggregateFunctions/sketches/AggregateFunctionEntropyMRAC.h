#pragma once

#include <AggregateFunctions/IAggregateFunction.h>
#include <DataTypes/DataTypesNumber.h>
#include <Columns/ColumnVector.h>
#include <Columns/ColumnArray.h>
#include <Common/assert_cast.h>

#include <cmath>

namespace DB
{

//Float64 median(std::vector<Float64> values)
//{
//    const size_t & n = values.size();
//    if (n % 2 == 0)
//    {
//		std::nth_element(values.begin(), values.begin() + n / 2, values.end());
//        std::nth_element(values.begin(), values.begin() + (n - 1) / 2, values.end()); 	
//		return (values[(n-2) / 2] + values[n / 2]) / 2.0;
//    }
//    else
//    {
//		std::nth_element(values.begin(), values.begin() + n / 2, values.end());
//		return values[(n-1) / 2];
//    }
//}

struct MRACData
{
    using sketch_t = uint32_t;
    std::vector<std::vector<sketch_t>> sketch_array;
    uint64_t sketch_total;
    unsigned int levels, rows, width;

    void add(const IColumn ** columns, size_t row_num, Poco::Logger * /*log*/)
    {
        //LOG_WARNING(log, "'add' called in MRACData with row_num {}", row_num);
        const auto & column = assert_cast<const ColumnArray &>(*columns[0]);
        Field row_field;
        column.get(row_num, row_field);

        if (row_num == 0)
        {
            Array & arguments = row_field.get<const Array &>();
            sketch_total = arguments[0].get<const uint64_t &>();
            levels = static_cast<const unsigned int &>(arguments[1].get<const uint64_t &>());
            rows = static_cast<const unsigned int &>(arguments[2].get<const uint64_t &>());
            width = static_cast<const unsigned int &>(arguments[3].get<const uint64_t &>());
            sketch_array.resize(levels);
            assert(rows == 1);
            for(uint32_t i = 0; i < levels; i++)
            {
                sketch_array[i].resize(width);
            }
        }
        else
        {
            row_num -= 1;
            unsigned int _level = static_cast<unsigned int>(row_num);
            for(uint32_t i = 0; i < width; i++)
            {
                // TODO: Why is column an Array(long)?
                Field & column_field = row_field.get<const Array &>()[i];
                sketch_array[_level][i] = static_cast<const sketch_t &>(column_field.get<const long &>());
            }
        }
    }

    Float64 get(Poco::Logger * /*log*/) const
    {
        Float64 entropy_estimate = 0;

        for(uint32_t i = 0; i < levels; i++)
        {
            for(uint32_t j = 0; j < width; j++)
            {
                // no need of abs as sketch_t is unsigned
                const uint32_t & counter = sketch_array[i][j];
                const Float64 & counter_prob = static_cast<Float64>(counter) / static_cast<Float64>(sketch_total);
                if (counter_prob != 0)
                {
                    entropy_estimate += counter_prob * log2(counter_prob);
                }
            }
        }
        entropy_estimate *= -1.0;
        
        return entropy_estimate;
    }
};

class AggregateFunctionEntropyMRAC final : public IAggregateFunctionDataHelper<MRACData, AggregateFunctionEntropyMRAC>
{
private:
    size_t num_args;
    Poco::Logger * log;

public:
    explicit AggregateFunctionEntropyMRAC(const DataTypes & argument_types_)
        : IAggregateFunctionDataHelper<MRACData, AggregateFunctionEntropyMRAC>(argument_types_, {}, createResultType())
        , num_args(argument_types_.size())
        , log(&Poco::Logger::get("AggregateFunctionEntropyMRAC"))
    {
        LOG_WARNING(log, "num_args {}", num_args);
        for(size_t i = 0; i < num_args; i++)
            LOG_WARNING(log, "arg {} type {}", i, argument_types_[i]->getName());
    }

    String getName() const override
    {
        return "AggregateFunctionEntropyMRAC_sketch";
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
        auto & column = assert_cast<ColumnVector<Float64> &>(to);
        column.getData().push_back(this->data(place).get(log));
    }
};

}

