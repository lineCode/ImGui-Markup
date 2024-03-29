#include "impch.h"
#include "imgui_markup/objects/panel.h"

namespace imgui_markup
{

Panel::Panel(std::string id, Object* parent)
    : Object("Panel", id, parent)
{
    this->AddAttribute("title", &this->title_);
    this->AddAttribute("position", &this->global_position_);
    this->AddAttribute("size", &this->size_);
}

Panel& Panel::operator=(const Panel& other)
{
    for (auto& child : this->child_objects_)
        child->SetParent(other.parent_);

    return *this;
}

void Panel::Update()
{
    if (this->init_panel_attributes_)
        this->InitPanelAttributes();

    if (!ImGui::Begin(this->title_))
    {
        ImGui::End();
        this->is_hovered_ = false;
        this->size_ = Float2();
        return;
    }

    this->is_hovered_ =
        ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
    this->size_ = ImGui::GetWindowSize();
    this->global_position_ = ImGui::GetWindowPos();

    for (auto& child : this->child_objects_)
    {
        if (!child)
            continue;

        child->SetPosition(ImGui::GetCursorPos(), this->global_position_);
        child->Update();
    }

    ImGui::End();
}

void Panel::InitPanelAttributes()
{
    if (this->global_position_.value_changed_)
        ImGui::SetNextWindowPos(this->global_position_);
    if (this->size_.value_changed_)
        ImGui::SetNextWindowSize(this->size_);

    this->init_panel_attributes_ = false;
}

bool Panel::Validate(std::string& error_message) const
{
    if (!this->parent_)
        return true;

    if (this->parent_->GetType() == "GlobalObject")
        return true;

    error_message = "Object of type \"Panel\" can only be created inside the "
                    "global file scope";

    return false;
}

bool Panel::OnProcessEnd(std::string& error_message)
{
    if (this->title_.value.empty())
        this->title_ = this->id_.empty() ? "unknown" : this->id_;

    return true;
}

}  // namespace imgui_markup
