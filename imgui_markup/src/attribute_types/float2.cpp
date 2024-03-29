#include "impch.h"
#include "imgui_markup/attribute_types/float2.h"

namespace imgui_markup
{

Float2::Float2()
    : Attribute(AttributeType::kFloat2)
{ }

Float2::Float2(float x, float y)
    : Attribute(AttributeType::kFloat2), x(x), y(y)
{ }

Float2::Float2(ImVec2 vec)
    : Attribute(AttributeType::kFloat2), x(vec.x), y(vec.y)
{ }

std::string Float2::ToString() const
{
    return std::to_string(this->x) + ", " + std::to_string(this->y);
}

bool Float2::IMPL_LoadValue(const Float2& value_in)
{
    this->x = value_in.x;
    this->y = value_in.y;
    return true;
}

bool Float2::IMPL_LoadValue(const String& value_in)
{
    std::vector<std::string> segments =
        internal::utils::SplitString(value_in, ',');

    if (segments.size() != 2)
        return false;

    if (!this->x.LoadValue(String(segments[0])))
        return false;

    // Y:
    if (!this->y.LoadValue(String(segments[1])))
        return false;

    return true;
}

}  // namespace imgui_markup
