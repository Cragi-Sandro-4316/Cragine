#include "CraginePCH.h"

#include "layers/Layer.h"

namespace Cragine {
    Layer::Layer(const std::string& debugName)
        : debugName(debugName) {}

    Layer::~Layer() {}
}