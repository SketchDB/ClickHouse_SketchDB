#include <Sketches/SketchFactory.h>
//#include <Parsers/ASTFunction.h>
#include <Common/Exception.h>
//#include <Common/StringUtils/StringUtils.h>
//#include <IO/WriteHelpers.h>

namespace DB
{

namespace ErrorCodes
{
    extern const int UNKNOWN_SKETCH;
    extern const int LOGICAL_ERROR;
}

void SketchFactory::registerSketch(const std::string & name, CreatorFn creator_fn)
{
    if (!sketches.emplace(name, Creator{std::move(creator_fn)}).second)
        throw Exception(ErrorCodes::LOGICAL_ERROR, "SketchFactory: the sketch name '{}' is not unique", name);
}

SketchPtr SketchFactory::get(const String & name, const Arguments & arguments) const
{
    auto it = sketches.find(name);
    if (it == sketches.end())
    {
        throw Exception(ErrorCodes::UNKNOWN_SKETCH, "Unknown sketch {}", name);
    }

    //Arguments arguments{
    //    .
    //    .comment = comment};

    auto res = sketches.at(name).creator_fn(arguments);
    
    return res;
}

SketchFactory & SketchFactory::instance()
{
    static SketchFactory ret;
    return ret;
}

}
