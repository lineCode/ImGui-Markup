#include "ilpch.h"
#include "imgui_layer/parser/parser.h"

#include <iostream>

namespace gui
{

/* Nodes */
ParserNode::ParserNode(ParserNodeType type, ParserPosition position)
    : type(type), position(position)
{ }

ParserObjectNode::ParserObjectNode(
    std::string object_name,
    std::string object_id,
    ParserPosition position)
    : ParserNode(ParserNodeType::kObjectNode, position),
      object_name(object_name), object_id(object_id)
{ }

ParserStringNode::ParserStringNode(
    std::string value,
    ParserPosition position)
    : ParserNode(ParserNodeType::kStringNode, position),
      value(value)
{ }

ParserNumberNode::ParserNumberNode(
    std::string value,
    ParserPosition position)
    : ParserNode(ParserNodeType::kNumberNode, position),
      value(value)
{ }

ParserVectorNode::ParserVectorNode(
    std::string value,
    ParserPosition position)
    : ParserNode(ParserNodeType::kVectorNode, position),
      value(value)
{ }

ParserAttributeAssignNode::ParserAttributeAssignNode(
    std::string attribute_name,
    std::shared_ptr<ParserNode> value_node,
    ParserPosition position)
    : ParserNode(ParserNodeType::kAttributeAssignNode, position),
      attribute_name(attribute_name), value_node(value_node)
{ }

ParserAttributeAccessNode::ParserAttributeAccessNode(
    std::string attribute_name,
    ParserPosition position)
    : ParserNode(ParserNodeType::kAttributeAccessNode, position),
      attribute_name(attribute_name)
{ }

/* Parser */
ParserResult Parser::ParseFile(const std::string file, GlobalObject& dest)
{
    this->Reset();

    try
    {
        this->lexer_.InitFile(file);

        std::shared_ptr<ParserNode> root_node = std::make_shared<ParserNode>(
            ParserNodeType::kRootNode, ParserPosition({file}, "", 0, 0, 0));

        this->ProcessTokens(root_node);
    }
    catch(const LexerException& e)
    {
        return ParserResult(e.type, e.message, e.token.position);
    }
    catch (const ParserException& e)
    {
        return ParserResult(e.type, e.message, e.token.position);
    }

    return ParserResult(ParserResultType::kSuccess);
}

void Parser::Reset()
{
    this->lexer_.Reset();
}

void Parser::ProcessTokens(std::shared_ptr<ParserNode> parent_node)
{
    if (!parent_node)
        return;

    LexerToken token;
    while (this->lexer_.GetNextToken(token))
    {
        if (TokenIsBlockEnd(*parent_node))
            return;

        if (TokenIsObjectNode())
            this->CreateObjectNode(*parent_node);
        else if (TokenIsAttributeAssignNode())
            this->CreateAttributeAssignNode(*parent_node);
        else
            throw UndefinedTokenSequence(token);
    }
}

bool Parser::TokenIsBlockEnd(const ParserNode& current_node)
{
    LexerToken token = this->lexer_.LookAhead(0);
    if (token.type == LexerTokenType::kCBracketClose)
    {
        if (current_node.type == ParserNodeType::kRootNode)
            throw UnexpectedBlockEnd(token);

        return true;
    }

    return false;
}

/* Object node */
bool Parser::TokenIsObjectNode()
{
    const LexerToken current_token = this->lexer_.LookAhead(0);
    const LexerToken next_token    = this->lexer_.LookAhead(1);

    if (current_token.type != LexerTokenType::kData)
        return false;

    if (next_token.type == LexerTokenType::kCBracketOpen ||
        next_token.type == LexerTokenType::kColon)
    {
        return true;
    }

    return false;
}

void Parser::CreateObjectNode(ParserNode& parent_node)
{
    const std::string name = this->lexer_.LookAhead(0).data;
    std::string id = "";

    const size_t start_position = this->lexer_.LookAhead(0).position.start;
    size_t end_position         = this->lexer_.LookAhead(0).position.end;

    LexerToken token;
    if (!this->lexer_.GetNextToken(token))
        throw UnexpectedEndOfFile(this->lexer_.LookAhead(0));

    // Check if the ID is set
    if (token.type == LexerTokenType::kColon)
    {
        if (!this->lexer_.GetNextToken(token))
            throw UnexpectedEndOfFile(this->lexer_.LookAhead(0));
        if (token.type != LexerTokenType::kData)
            throw ObjectIDWrongValueType(token);

        id = token.data;
        end_position = token.position.end;

        // Move one token, where we expect the start of the object block
        if (!this->lexer_.GetNextToken(token))
            throw UnexpectedEndOfFile(this->lexer_.LookAhead(0));
    }

    if (token.type != LexerTokenType::kCBracketOpen)
        throw ExpectedStartOfBlock(token);

    ParserPosition object_position = token.position;
    object_position.start = start_position;
    object_position.end   = end_position;

    std::shared_ptr<ParserNode> node =
        std::make_shared<ParserObjectNode>(name, id, object_position);

    if (!node)
        throw UnableToCreateObjectNode(token);

    this->ProcessTokens(node);

    parent_node.child_nodes.push_back(node);
}

/* Attribute assign node */
bool Parser::TokenIsAttributeAssignNode()
{
    const LexerToken current_token = this->lexer_.LookAhead(0);
    const LexerToken next_token    = this->lexer_.LookAhead(1);

    if (current_token.type == LexerTokenType::kData &&
        next_token.type    == LexerTokenType::kEqual)
    {
        return true;
    }

    return false;
}

void Parser::CreateAttributeAssignNode(ParserNode& parent_node)
{
    const std::string name = this->lexer_.LookAhead(0).data;

    const size_t start_position = this->lexer_.LookAhead(0).position.start;

    LexerToken token;
    if (!this->lexer_.GetNextToken(token))
        throw UnexpectedEndOfFile(this->lexer_.LookAhead(0));

    if (token.type != LexerTokenType::kEqual)
        throw ExpectedEqualSymbol(token);

    // Skip equal symbol
    if (!this->lexer_.GetNextToken(token))
        throw UnexpectedEndOfFile(this->lexer_.LookAhead(0));

    std::shared_ptr<ParserNode> value_node;

    if (this->TokenIsStringNode())
        value_node = this->CreateStringNode();
    else if (this->TokenIsNumberNode())
        value_node = this->CreateNumberNode();
    else if (this->TokenIsVectorNode())
        value_node = this->CreateVectorNode();
    else if(this->TokenIsAttributeAccessNode())
        value_node = this->CreateAttributeAccessNode();
    else
        throw ValueNodeWrongType(token);

    ParserPosition position = value_node->position;
    position.start = start_position;

    std::shared_ptr<ParserNode> node =
        std::make_shared<ParserAttributeAssignNode>(name, value_node, position);

    if (!node)
        throw UnableToCreateAttributeAssignNode(token);

    parent_node.child_nodes.push_back(node);
}

/* String node */
bool Parser::TokenIsStringNode()
{
    return this->lexer_.LookAhead(0).type == LexerTokenType::kString
                ? true : false;
}

std::shared_ptr<ParserStringNode> Parser::CreateStringNode()
{
    const LexerToken token = this->lexer_.LookAhead(0);
    if (token.type != LexerTokenType::kString)
        throw ValueNodeWrongType(token);

    std::shared_ptr<ParserStringNode> node =
        std::make_shared<ParserStringNode>(token.data, token.position);

    if (!node)
        throw UnableToCreateStringNode(token);

    return node;
}

/* Number node */
bool Parser::TokenIsNumberNode()
{
    return this->lexer_.LookAhead(0).type == LexerTokenType::kNumber
                ? true : false;
}

std::shared_ptr<ParserNumberNode> Parser::CreateNumberNode()
{
    const LexerToken token = this->lexer_.LookAhead(0);
    if (token.type != LexerTokenType::kNumber)
        throw ValueNodeWrongType(token);

    std::shared_ptr<ParserNumberNode> node =
        std::make_shared<ParserNumberNode>(token.data, token.position);

    if (!node)
        throw UnableToCreateNumberNode(token);

    return node;
}

/* Vector node */
bool Parser::TokenIsVectorNode()
{
    return this->lexer_.LookAhead(0).type == LexerTokenType::kBracketOpen
                ? true : false;
}

// TODO: Check if the attribute types are correct/valid
std::shared_ptr<ParserVectorNode> Parser::CreateVectorNode()
{
    std::string data = "(";

    LexerToken token = this->lexer_.LookAhead(0);

    if (token.type != LexerTokenType::kBracketOpen)
        throw ValueNodeWrongType(token);

    const size_t start_position = token.position.start;

    while (token.type != LexerTokenType::kBracketClose)
    {
        if (!this->lexer_.GetNextToken(token))
            throw UnexpectedEndOfVector(token);
        if (token.type == LexerTokenType::kComma)
        {
            data += ',';
            continue;
        }

        data += token.data;
    }
    data += ')';

    ParserPosition position = token.position;
    position.start = start_position;

    std::shared_ptr<ParserVectorNode> node =
        std::make_shared<ParserVectorNode>(data, position);

    if (!node)
        throw UnableToCreateVectorNode(token);

    return node;
}

/* Attribute access node */
bool Parser::TokenIsAttributeAccessNode()
{
    const LexerToken current_token = this->lexer_.LookAhead(0);

    if (current_token.type == LexerTokenType::kData)
        return true;

    return false;
}

std::shared_ptr<ParserAttributeAccessNode> Parser::CreateAttributeAccessNode()
{
    const LexerToken token = this->lexer_.LookAhead(0);
    if (token.type != LexerTokenType::kData)
        throw ValueNodeWrongType(token);

    std::shared_ptr<ParserAttributeAccessNode> node =
        std::make_shared<ParserAttributeAccessNode>(token.data, token.position);

    if (!node)
        throw UnableToCreateAttributeAccessNode(token);

    return node;
}

}  // namespace gui
