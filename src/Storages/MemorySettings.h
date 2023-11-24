#pragma once

#include <Core/BaseSettings.h>


namespace DB
{
class ASTStorage;


#define MEMORY_SETTINGS(M, ALIAS) \
    M(Bool, compress, false, "Compress data in memory", 0) \
    M(String, sketch_dp_configuration, "", "Sketch data plane configuration", 0) \
    M(String, sketch_cp_configuration, "", "Sketch control plane configuration", 0) \

DECLARE_SETTINGS_TRAITS(memorySettingsTraits, MEMORY_SETTINGS)


/** Settings for the Memory engine.
  * Could be loaded from a CREATE TABLE query (SETTINGS clause).
  */
struct MemorySettings : public BaseSettings<memorySettingsTraits>
{
    void loadFromQuery(ASTStorage & storage_def);
};

}

