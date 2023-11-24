#include <Sketches/ISketch.h>

namespace DB
{

ISketch::ISketch(String _name)
: log(&Poco::Logger::get(_name))
{
    name = _name;
}

}
