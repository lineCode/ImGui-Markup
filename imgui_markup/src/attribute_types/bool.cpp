#include "impch.h"
#include "imgui_markup/attribute_types/bool.h"

namespace imgui_markup
{

Bool::Bool()
    : Attribute(AttributeType::kBool)
{ }

Bool::Bool(const bool b)
    : Attribute(AttributeType::kBool), value(b)
{ }

std::string Bool::ToString() const
{
    return internal::utils::BoolToString(this->value);
}

bool Bool::IMPL_LoadValue(const Bool& value_in)
{
    this->value = value_in;
    return true;
}

bool Bool::IMPL_LoadValue(const String& value_in)
{
    return internal::utils::StringToBool(value_in, &this->value);
}

}  // namespace imgui_markup
