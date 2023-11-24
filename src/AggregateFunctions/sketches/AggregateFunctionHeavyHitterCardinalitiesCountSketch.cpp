#include <AggregateFunctions/AggregateFunctionFactory.h>
#include <AggregateFunctions/sketches/AggregateFunctionHeavyHitterCardinalitiesCountSketch.h>
#include <AggregateFunctions/FactoryHelpers.h>
#include <AggregateFunctions/Helpers.h>

namespace DB
{
struct Settings;

namespace ErrorCodes
{
    extern const int NUMBER_OF_ARGUMENTS_DOESNT_MATCH;
}

namespace
{

AggregateFunctionPtr createAggregateFunctionHeavyHitterCardinalitiesCountSketch(
    const std::string & name, const DataTypes & argument_types, const Array & parameters, const Settings *)
{
    assertNoParameters(name, parameters);
    // TODO: need one parameters which is heavy hitter size
    if (argument_types.empty())
        throw Exception(ErrorCodes::NUMBER_OF_ARGUMENTS_DOESNT_MATCH,
                        "Incorrect number of arguments for aggregate function {}", name);

    return std::make_shared<AggregateFunctionHeavyHitterCardinalitiesCountSketch>(argument_types);
}

}

void registerAggregateFunctionHeavyHitterCardinalitiesCountSketch(AggregateFunctionFactory & factory)
{
    factory.registerFunction("heavy_hitter_cardinalities_sketch_cs", createAggregateFunctionHeavyHitterCardinalitiesCountSketch);
}

}

