#include "impch.h"
#include "imgui_markup/parser/parser.h"

#include <iostream>

namespace imgui_markup::internal::parser
{

/* Parser */
ParserResult Parser::ParseFile(const std::string file, GlobalObject& dest)
{
    this->Reset();

    try
    {
        dest.Reset();

        this->lexer_.InitFile(file);

        std::shared_ptr<ParserNode> root_node = std::make_shared<ParserNode>(
            ParserNodeType::kRootNode, ParserPosition({file}, "", 0, 0, 0));

        this->ProcessTokens(root_node);

        this->interpreter_.ConvertNodeTree(root_node, dest);
    }
    catch(const LexerException& e)
    {
        return ParserResult(e.type, e.message, e.token.position);
    }
    catch (const ParserException& e)
    {
        return ParserResult(e.type, e.message, e.token.position);
    }
    catch (const InterpreterException& e)
    {
        return ParserResult(e.type, e.message, e.node.position);
    }

    return ParserResult(ParserResultType::kSuccess);
}

void Parser::Reset()
{
    this->lexer_.Reset();
    this->interpreter_.Reset();
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
    LexerToken token = this->lexer_.LookAhead(0);
    ParserPosition object_position = token.position;

    const std::string type = token.data;
    std::string id = "";

    size_t end_position = this->lexer_.LookAhead(0).position.end;

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

        end_position = this->lexer_.LookAhead(0).position.end;

        // Move one token, where we expect the start of the object block
        if (!this->lexer_.GetNextToken(token))
            throw UnexpectedEndOfFile(this->lexer_.LookAhead(0));
    }

    if (token.type != LexerTokenType::kCBracketOpen)
        throw ExpectedStartOfBlock(token);

    object_position.end = end_position;

    std::shared_ptr<ParserNode> node =
        std::make_shared<ParserObjectNode>(type, id, object_position);

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
    else if (this->TokenIsIntNode())
        value_node = this->CreateIntNode();
    else if (this->TokenIsFloatNode())
        value_node = this->CreateFloatNode();
    else if (this->TokenIsBoolNode())
        value_node = this->CreateBoolNode();
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
bool Parser::TokenIsIntNode()
{
    return this->lexer_.LookAhead(0).type == LexerTokenType::kInt
                ? true : false;
}

std::shared_ptr<ParserIntNode> Parser::CreateIntNode()
{
    const LexerToken token = this->lexer_.LookAhead(0);
    if (token.type != LexerTokenType::kInt)
        throw ValueNodeWrongType(token);

    std::shared_ptr<ParserIntNode> node =
        std::make_shared<ParserIntNode>(token.data, token.position);

    if (!node)
        throw UnableToCreateNumberNode(token);

    return node;
}

/* Float node */
bool Parser::TokenIsFloatNode()
{
    return this->lexer_.LookAhead(0).type == LexerTokenType::kFloat
                ? true : false;
}

std::shared_ptr<ParserFloatNode> Parser::CreateFloatNode()
{
    const LexerToken token = this->lexer_.LookAhead(0);
    if (token.type != LexerTokenType::kFloat)
        throw ValueNodeWrongType(token);

    std::shared_ptr<ParserFloatNode> node =
        std::make_shared<ParserFloatNode>(token.data, token.position);

    if (!node)
        throw UnableToCreateNumberNode(token);

    return node;
}

/* Bool node */
bool Parser::TokenIsBoolNode()
{
    return this->lexer_.LookAhead(0).type == LexerTokenType::kBool
                ? true : false;
}

std::shared_ptr<ParserBoolNode> Parser::CreateBoolNode()
{
    const LexerToken token = this->lexer_.LookAhead(0);
    if (token.type != LexerTokenType::kBool)
        throw ValueNodeWrongType(token);

    std::shared_ptr<ParserBoolNode> node =
        std::make_shared<ParserBoolNode>(token.data, token.position);

    if (!node)
        throw UnableToCreateBoolNode(token);

    return node;
}

/* Vector node */
bool Parser::TokenIsVectorNode()
{
    return this->lexer_.LookAhead(0).type == LexerTokenType::kBracketOpen
                ? true : false;
}

std::shared_ptr<ParserVectorNode> Parser::CreateVectorNode()
{
    LexerToken token = this->lexer_.LookAhead(0);

    if (token.type != LexerTokenType::kBracketOpen)
        throw ValueNodeWrongType(token);

    const size_t start_position = token.position.start;

    std::shared_ptr<ParserVectorNode> node =
        std::make_shared<ParserVectorNode>(token.position);

    if (!node)
        throw UnableToCreateVectorNode(token);

    while (token.type != LexerTokenType::kBracketClose)
    {
        if (!this->lexer_.GetNextToken(token))
            throw UnexpectedEndOfFile(token);
        if (token.type == LexerTokenType::kComma)
        {
            if (this->lexer_.LookAhead(1).type == LexerTokenType::kComma ||
                this->lexer_.LookAhead(1).type == LexerTokenType::kBracketClose)
            {
                throw MissingVectorValue(token);
            }
            continue;
        }
        if (token.type == LexerTokenType::kBracketClose)
            break;

        std::shared_ptr<ParserNode> value_node;

        if (this->TokenIsStringNode())
            value_node = this->CreateStringNode();
        else if (this->TokenIsIntNode())
            value_node = this->CreateIntNode();
        else if (this->TokenIsFloatNode())
            value_node = this->CreateFloatNode();
        else if(this->TokenIsAttributeAccessNode())
            value_node = this->CreateAttributeAccessNode();
        else
            throw ValueNodeWrongType(token);

        node->child_nodes.push_back(value_node);
    }

    ParserPosition position = token.position;
    position.start = start_position;
    node->position = position;

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

}  // namespace imgui_markup::internal::parser
