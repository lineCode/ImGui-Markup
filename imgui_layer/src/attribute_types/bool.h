#ifndef IMGUI_LAYER_SRC_ATTRIBUTE_TYPES_BOOL_H_
#define IMGUI_LAYER_SRC_ATTRIBUTE_TYPES_BOOL_H_

#include "attribute_types/attribute_type.h"

namespace gui
{

struct Bool : public AttributeType
{
    bool LoadValue(std::string value);
    std::string ToString();
    bool value;
};

}  // namespace gui

#endif  // IMGUI_LAYER_SRC_ATTRIBUTE_TYPES_BOOL_H_
