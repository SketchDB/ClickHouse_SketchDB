#include <AggregateFunctions/AggregateFunctionFactory.h>
#include <AggregateFunctions/sketches/AggregateFunctionCardinalityLC.h>
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

AggregateFunctionPtr createAggregateFunctionCardinalityLC(
    const std::string & name, const DataTypes & argument_types, const Array & parameters, const Settings *)
{
    assertNoParameters(name, parameters);
    if (argument_types.empty())
        throw Exception(ErrorCodes::NUMBER_OF_ARGUMENTS_DOESNT_MATCH,
                        "Incorrect number of arguments for aggregate function {}", name);

    return std::make_shared<AggregateFunctionCardinalityLC>(argument_types);
}

}

void registerAggregateFunctionCardinalityLC(AggregateFunctionFactory & factory)
{
    factory.registerFunction("cardinality_sketch_lc", createAggregateFunctionCardinalityLC);
}

}


