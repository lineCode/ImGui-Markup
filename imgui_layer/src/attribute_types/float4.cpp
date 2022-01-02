#include "ilpch.h"
#include "imgui_layer/attribute_types/float4.h"

namespace gui
{

Float4::Float4()
    : Attribute(AttributeType::kFloat4)
{ }

Float4::Float4(float x, float y, float z, float w)
    : Attribute(AttributeType::kFloat4), x(x), y(y), z(z), w(w)
{ }

Float4::Float4(ImVec4 vec)
    : Attribute(AttributeType::kFloat4), x(vec.x), y(vec.y), z(vec.z), w(vec.w)
{ }

std::string Float4::ToString() const
{
    return std::to_string(this->x) + "," +
           std::to_string(this->y) + "," +
           std::to_string(this->z) + "," +
           std::to_string(this->w);
}

bool Float4::IMPL_LoadValue(std::string value_in)
{
    std::vector<std::string> segments = utils::SplitString(value_in, ',');

    if (segments.size() != 4)
        return false;

    // X:
    if (!this->x.LoadValue(segments[0]))
        return false;

    // Y:
    if (!this->y.LoadValue(segments[1]))
        return false;

    // z:
    if (!this->z.LoadValue(segments[2]))
        return false;

    // w:
    if (!this->w.LoadValue(segments[3]))
        return false;

    return true;
}

}  // namespace gui
