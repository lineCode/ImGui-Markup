#ifndef IMGUI_LAYER_SRC_OBJECTS_PANEL_H_
#define IMGUI_LAYER_SRC_OBJECTS_PANEL_H_

#include "objects/object.h"

#include "attribute_types/float2.h"

#include "imgui.h"

namespace gui
{

class IMGUI_LAYER_API Panel : public Object
{
public:
    // Constructor/Destructor
    Panel(std::string id, Object* parent);

    // Functions
    void Update();

    Float2 size_;
    Float2 pos_;
};

}  // namespace gui

#endif  // IMGUI_LAYER_SRC_OBJECTS_PANEL_H_
