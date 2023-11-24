#pragma once

#include <Sketches/ISketch.h>

#include <boost/noncopyable.hpp>

#include <unordered_map>
#include <functional>

namespace DB
{

class SketchFactory : private boost::noncopyable
{
public:

    static SketchFactory & instance();

    struct Arguments
    {
        const unsigned int & levels;
        const unsigned int & rows;
        const unsigned int & width;
    };

    using CreatorFn = std::function<SketchPtr(const Arguments & arguments)>;
    struct Creator
    {
        CreatorFn creator_fn;
    };

    using Sketches = std::unordered_map<std::string, Creator>;

    SketchPtr get(const String & name, const Arguments & arguments) const;

    void registerSketch(const std::string & name, CreatorFn creator_fn);

    const Sketches & getAllSketches() const
    {
        return sketches;
    }

    //std::vector<String> getAllRegisteredNames() const 
    //{
    //    std::vector<String> result;
    //    auto getter = [](const auto & pair) { return pair.first; };
    //    std::transform(sketches.begin(), sketches.end(), std::back_inserter(result), getter);
    //    return result;
    //}

private:
    Sketches sketches;
};

}
