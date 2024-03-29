#include "impch.h"
#include "imgui_markup/parser/interpreter.h"

#include "objects/common/object_list.h"

namespace imgui_markup::internal::parser
{

/* Interpreter */
void Interpreter::ConvertNodeTree(
    const std::shared_ptr<ParserNode>& root_node, GlobalObject& dest)
{
    this->Reset();

    this->InitObjectReference(dest, *root_node.get());
    this->ProcessNodes(*root_node.get(), dest);

    dest.object_references_ = this->object_references_;
}

void Interpreter::Reset()
{
    this->object_references_.clear();
}

void Interpreter::InitObjectReference(
    Object& object, const ParserNode& node)
{
    if (object.GetID().empty())
        return;

    std::string full_id = object.GetID();

    Object* parent = &object;
    while (true)
    {
        parent = parent->GetParent();

        if (!parent)
            break;
        if (parent->GetID().empty())
            continue;
        if (parent->GetID() == "global")
            continue;

        full_id.insert(0, parent->GetID() + '.');
    }

    if (this->object_references_.find(full_id) !=
        this->object_references_.end())
    {
        throw ObjectIDAlreadyDefined(full_id, node);
    }

    this->object_references_.insert(
        std::pair<std::string, Object&>(full_id, object));
}

void Interpreter::ProcessNodes(
    const ParserNode& node, Object& parent_object)
{
    for (const auto& child : node.child_nodes)
    {
        switch (child->type)
        {
        case ParserNodeType::kObjectNode:
            this->ProcessObjectNode(*child.get(), parent_object);
            break;
        case ParserNodeType::kAttributeAssignNode:
            this->ProcessAttributeAssignNode(*child.get(), parent_object);
            break;
        default:
            throw WrongBaseNode(*child.get());
        }
    }
}

void Interpreter::ProcessObjectNode(
    const ParserNode& node_in, Object& parent_object)
{
    if (node_in.type != ParserNodeType::kObjectNode)
        throw InternalWrongNodeType(node_in);

    ParserObjectNode& node = (ParserObjectNode&)node_in;

    std::shared_ptr<Object> object = ObjectList::CreateObject(
        node.object_type, node.object_id, &parent_object);

    if (!object)
        throw UndefinedObjectType(node);

    std::string error_message;
    if (!object->Validate(error_message))
        throw ObjectIsNotValid(error_message, node);

    parent_object.AddChild(object);

    this->InitObjectReference(*object.get(), node);

    if (!object->OnProcessStart(error_message))
        throw ObjectIsNotValid(error_message, node);

    this->ProcessNodes(node_in, *object.get());

    if (!object->OnProcessEnd(error_message))
        throw ObjectIsNotValid(error_message, node);
}

void Interpreter::ProcessAttributeAssignNode(
    const ParserNode& node_in, Object& parent_object)
{
    if (node_in.type != ParserNodeType::kAttributeAssignNode)
        throw InternalWrongNodeType(node_in);

    ParserAttributeAssignNode& node = (ParserAttributeAssignNode&)node_in;

    if (!node.value_node)
        throw MissingAttributeValue(node);

    std::shared_ptr<Attribute> value =
        this->ProcessValueNode(*node.value_node.get(), parent_object);

    if (!value)
        throw AttributeConversionError("Undefined", "Undefined", node);

    Attribute* attribute = parent_object.GetAttribute(node.attribute_name);
    if (!attribute)
    {
        throw AttributeDoesNotExists(
            parent_object.GetType(), node.attribute_name, node);
    }

    if (!attribute->LoadValue(*value))
    {
        throw AttributeConversionError(this->AttributeTypeToString(*attribute),
            this->AttributeTypeToString(*value), value->ToString(), node);
    }
}

String Interpreter::ProcessStringNode(
    const ParserNode& node_in) const
{
    if (node_in.type != ParserNodeType::kStringNode)
            throw InternalWrongNodeType(node_in);

    ParserStringNode& node = (ParserStringNode&)node_in;

    return String(node.value);
}

Int Interpreter::ProcessIntNode(
    const ParserNode& node_in) const
{
    if (node_in.type != ParserNodeType::kIntNode)
            throw InternalWrongNodeType(node_in);

    ParserIntNode& node = (ParserIntNode&)node_in;

    Int value;
    if (!value.LoadValue(String(node.value)))
        throw AttributeConversionError("Int", node.value, node_in);

    return value;
}

Float Interpreter::ProcessFloatNode(
    const ParserNode& node_in) const
{
    if (node_in.type != ParserNodeType::kFloatNode)
            throw InternalWrongNodeType(node_in);

    ParserFloatNode& node = (ParserFloatNode&)node_in;

    Float value;
    if (!value.LoadValue(String(node.value)))
        throw AttributeConversionError("Float", node.value, node_in);

    return value;
}

Bool Interpreter::ProcessBoolNode(
    const ParserNode& node_in) const
{
    if (node_in.type != ParserNodeType::kBoolNode)
            throw InternalWrongNodeType(node_in);

    ParserBoolNode& node = (ParserBoolNode&)node_in;

    Bool value;
    if (!value.LoadValue(String(node.value)))
        throw AttributeConversionError("Bool", node.value, node_in);

    return value;
}

std::shared_ptr<Attribute> Interpreter::ProcessVectorNode(
    const ParserNode& node_in, Object& parent_object) const
{
    if (node_in.type != ParserNodeType::kVectorNode)
            throw InternalWrongNodeType(node_in);

    ParserVectorNode& node = (ParserVectorNode&)node_in;

    std::string value;
    size_t value_count = 0;

    for (const auto& child : node.child_nodes)
    {
        if (!value.empty())
            value += ',';

        value +=
            this->ProcessValueNode(*child.get(), parent_object)->ToString();

        value_count++;
    }

    if (value_count == 2)
    {
        Float2 final_value;
        if (!final_value.LoadValue(String(value)))
            throw AttributeConversionError("Float2", value, node_in);
        return std::make_shared<Float2>(final_value);
    }
    else if (value_count == 3)
    {
        Float3 final_value;
        if (!final_value.LoadValue(String(value)))
            throw AttributeConversionError("Float3", value, node_in);
        return std::make_shared<Float3>(final_value);
    }
    else if (value_count == 4)
    {
        Float4 final_value;
        if (!final_value.LoadValue(String(value)))
            throw AttributeConversionError("Float4", value, node_in);
        return std::make_shared<Float4>(final_value);
    }

    throw AttributeConversionError("Vector", value, node_in);
}

std::shared_ptr<Attribute> Interpreter::ProcessValueNode(
    const ParserNode& node, Object& parent_object) const
{
    switch (node.type)
    {
    case ParserNodeType::kStringNode:
        return std::make_shared<String>(this->ProcessStringNode(node));
    case ParserNodeType::kIntNode:
        return std::make_shared<Int>(this->ProcessIntNode(node));
    case ParserNodeType::kFloatNode:
        return std::make_shared<Float>(this->ProcessFloatNode(node));
    case ParserNodeType::kBoolNode:
        return std::make_shared<Bool>(this->ProcessBoolNode(node));
    case ParserNodeType::kVectorNode:
        return this->ProcessVectorNode(node, parent_object);
    case ParserNodeType::kAttributeAccessNode:
        return std::shared_ptr<Attribute>(
            &this->ProcessAttributeAccessNode(node, parent_object),
                [](Attribute*){});  // Used that the pointer does not delete
                                    // the attribute. This will be changed
                                    // when references are implemented.
    default:
        throw UnknownAttributeValueType(node);
    }
}

Attribute& Interpreter::ProcessAttributeAccessNode(
    const ParserNode& node_in, Object& parent_object) const
{
    if (node_in.type != ParserNodeType::kAttributeAccessNode)
            throw InternalWrongNodeType(node_in);

    ParserAttributeAccessNode& node = (ParserAttributeAccessNode&)node_in;

    std::string attribute = node.attribute_name;

    // We will assume a reference to a attribute of the current object
    // if the attribute name does not contain '.'
    if (attribute.find('.') == std::string::npos)
        return this->GetAttributeFromObject(attribute, parent_object, node);

    return this->GetAttribtueFromObjectReference(attribute, node);
}


Attribute& Interpreter::GetAttributeFromObject(
    const std::string attribute_name,
    Object& object,
    const ParserNode& node) const
{
    Attribute* attribute = object.GetAttribute(attribute_name);
    if (!attribute)
        throw AttributeDoesNotExists(object.GetType(), attribute_name, node);

    return *attribute;
}

Attribute& Interpreter::GetAttribtueFromObjectReference(
    std::string attribute, const ParserNode& node) const
{
    std::string object_id =
        this->GetObjectNameFromAttributeReferenceString(attribute, node);

    if (object_id.size() >= attribute.size())
        throw NoAttributeSpecified(node);

    attribute = attribute.substr(object_id.size() + 1);

    if (this->object_references_.find(object_id) ==
        this->object_references_.end())
    {
        throw ObjectIsNotDefined(object_id, node);
    }

    Object& object = this->object_references_.at(object_id);

    const std::string type = object.GetType();
    const std::string id   = object.GetID().empty() ?
                                "null" : object.GetID();

    Attribute* att = object.GetAttribute(attribute);
    if (!att)
        throw AttributeDoesNotExists(object.GetType(), attribute, node);

    return *att;
}

std::string Interpreter::GetObjectNameFromAttributeReferenceString(
    const std::string attribute, const ParserNode& node) const
{
    if (attribute.size() <= 3)
            throw InvalidObjectID(attribute, node);

    const std::vector<std::string> segments =
        utils::SplitString(attribute, '.');

    if (segments.empty())
        throw InvalidObjectID(attribute, node);

    std::string object_id;
    for (const auto& segment : segments)
    {
        std::string id = object_id;
        if (!id.empty())
            id += '.';

        id += segment;
        if (this->object_references_.find(id) == this->object_references_.end())
        {
            if (object_id.empty())
                object_id = id;
            break;
        }

        object_id = id;
    }

    if (this->object_references_.find(object_id) ==
        this->object_references_.end())
    {
        throw ObjectIsNotDefined(object_id, node);
    }

    return object_id;
}

std::string Interpreter::AttributeTypeToString(const Attribute& attribute) const
{
    return this->AttributeTypeToString(attribute.type);
}

std::string Interpreter::AttributeTypeToString(const AttributeType type) const
{
    switch (type)
    {
    case AttributeType::kBool:   return "Bool";
    case AttributeType::kFloat:  return "Float";
    case AttributeType::kFloat2: return "Float2";
    case AttributeType::kFloat3: return "Float3";
    case AttributeType::kFloat4: return "Float4";
    case AttributeType::kInt:    return "Int";
    case AttributeType::kString: return "String";
    default: return "Undefined";
    }
}

}  // namespace imgui_markup::internal::parser
