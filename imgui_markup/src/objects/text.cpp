#include "impch.h"
#include "imgui_markup/objects/text.h"

namespace imgui_markup
{

Text::Text(std::string id, Object* parent)
    : Object("Text", id, parent)
{
    this->AddAttribute("text", &this->text_);
    this->AddAttribute("color", &this->color_);
}

Text& Text::operator=(const Text& other)
{
    for (auto& child : this->child_objects_)
        child->SetParent(other.parent_);

    return *this;
}

void Text::Update()
{
    ImGui::SetCursorPos(this->draw_position_);

    if (this->color_.value_changed_)
        ImGui::TextColored(this->color_, "%s", this->text_.value.c_str());
    else
        ImGui::Text("%s", this->text_.value.c_str());

    this->is_hovered_ = ImGui::IsItemHovered();
    this->size_ = ImGui::GetItemRectSize();
}

}  // namespace imgui_markup
