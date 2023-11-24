#include <Sketches/registerSketches.h>
#include <Sketches/SketchFactory.h>

namespace DB
{

void registerCountMin(SketchFactory & factory);
void registerCountSketch(SketchFactory & factory);
void registerMRAC(SketchFactory & factory);
void registerLC(SketchFactory & factory);

void registerSketches()
{
    auto & factory = SketchFactory::instance();

    registerCountMin(factory);
    registerCountSketch(factory);
    registerMRAC(factory);
    registerLC(factory);
}

}
