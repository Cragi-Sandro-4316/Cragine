#include "layers/LayerStack.h"

#include "CraginePCH.h"


namespace Cragine {
    LayerStack::LayerStack() {}

    LayerStack::~LayerStack() {
        for (Layer* layer : layers)
            delete layer;
    }

    void LayerStack::pushLayer(Layer* layer) {
        layers.emplace(layers.begin() + layerInsertIndex, layer);
        layerInsertIndex++;
    }

    void LayerStack::pushOverlay(Layer* overlay) {
        layers.emplace_back(overlay);
    }

    void LayerStack::popLayer(Layer* layer) {
        auto it = std::find(layers.begin(), layers.end(), layer);
        if (it != layers.end()) {
            layers.erase(it);
            layerInsertIndex--;
        }

    }

    void LayerStack::popOverlay(Layer* overlay) {
        auto it = std::find(layers.begin(), layers.end(), overlay);
        if (it != layers.end()) 
            layers.erase(it);
    }



}
