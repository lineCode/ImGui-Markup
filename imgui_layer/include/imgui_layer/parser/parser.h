#ifndef IMGUI_LAYER_INCLUDE_IMGUI_LAYER_PARSER_PARSER_H_
#define IMGUI_LAYER_INCLUDE_IMGUI_LAYER_PARSER_PARSER_H_

#include "imgui_layer/parser/parser_result.h"
#include "imgui_layer/objects/global_object.h"
#include "imgui_layer/parser/lexer.h"

#include <string>
#include <vector>

namespace gui
{

/* Nodes */
enum class ParserNodeType
{
    kRootNode,
    kObjectNode,
    kAttributeAssignNode,
    kStringNode,
    kNumberNode,
    kVectorNode,
    kAttributeAccessNode
};

struct ParserNode
{
    ParserNode(ParserNodeType type, ParserPosition position);

    const ParserNodeType type;
    const ParserPosition position;
    std::vector<std::shared_ptr<ParserNode>> child_nodes;
};

struct ParserObjectNode : public ParserNode
{
    ParserObjectNode(std::string object_name, std::string object_id,
                     ParserPosition position);

    const std::string object_name;
    const std::string object_id;
};

struct ParserStringNode : public ParserNode
{
    ParserStringNode(std::string value, ParserPosition position);

    const std::string value;
};

struct ParserNumberNode : public ParserNode
{
    ParserNumberNode(std::string value, ParserPosition position);

    const std::string value;
};

struct ParserVectorNode : public ParserNode
{
    ParserVectorNode(std::string value, ParserPosition position);

    const std::string value;
};

struct ParserAttributeAssignNode : public ParserNode
{
    ParserAttributeAssignNode(std::string attribute_name,
                              std::shared_ptr<ParserNode> value_node,
                              ParserPosition position);

    const std::string attribute_name;
    const std::shared_ptr<ParserNode> value_node;
};

struct ParserAttributeAccessNode : public ParserNode
{
    ParserAttributeAccessNode(std::string attribute_name,
                              ParserPosition position);

    const std::string attribute_name;
};

/* Exceptions */
struct ParserException
{
    ParserException(
        std::string message, LexerToken token, ParserResultType type)
        : message(message), token(token), type(type)
    { }

    const std::string message;
    const LexerToken token;
    const ParserResultType type;
};

struct UnexpectedBlockEnd : public ParserException
{
    UnexpectedBlockEnd(LexerToken token)
        : ParserException("Unexpected end of block in global scope", token,
                         ParserResultType::kUnexpectedBlockEnd)
    { }
};

struct UndefinedTokenSequence : public ParserException
{
    UndefinedTokenSequence(LexerToken token)
        : ParserException("Undefined sequence of tokens, expected "
                          "object or attribute declaration", token,
                         ParserResultType::kUndefinedTokenSequence)
    { }
};

struct UnexpectedEndOfFile : public ParserException
{
    UnexpectedEndOfFile(LexerToken token)
        : ParserException("Unexpected end of file", token,
                         ParserResultType::kUnexpectedEndOfFile)
    { }
};

struct ObjectIDWrongValueType : public ParserException
{
    ObjectIDWrongValueType(LexerToken token)
        : ParserException("Wrong value type of the object id", token,
                         ParserResultType::kObjectIDWrongValueType)
    { }
};

struct ExpectedStartOfBlock : public ParserException
{
    ExpectedStartOfBlock(LexerToken token)
        : ParserException("Expected block starting with '{'", token,
                         ParserResultType::kExpectedStartOfBlock)
    { }
};

struct ExpectedEqualSymbol : public ParserException
{
    ExpectedEqualSymbol(LexerToken token)
        : ParserException("Unexpected symbol, expected '='", token,
                         ParserResultType::kExpectedEqualSymbol)
    { }
};

struct UnableToCreateObjectNode : public ParserException
{
    UnableToCreateObjectNode(LexerToken token)
        : ParserException("Unable to create object node", token,
                         ParserResultType::kUnableToCreateObjectNode)
    { }
};

struct UnableToCreateAttributeAssignNode : public ParserException
{
    UnableToCreateAttributeAssignNode(LexerToken token)
        : ParserException("Unable to create attribute assign node", token,
                         ParserResultType::kUnableToCreateAttributeAssignNode)
    { }
};

struct UnableToCreateStringNode : public ParserException
{
    UnableToCreateStringNode(LexerToken token)
        : ParserException("Unable to create string node", token,
                         ParserResultType::kUnableToCreateStringNode)
    { }
};

struct UnableToCreateNumberNode : public ParserException
{
    UnableToCreateNumberNode(LexerToken token)
        : ParserException("Unable to create number node", token,
                         ParserResultType::kUnableToCreateNumberNode)
    { }
};

struct UnableToCreateVectorNode : public ParserException
{
    UnableToCreateVectorNode(LexerToken token)
        : ParserException("Unable to create vector node", token,
                         ParserResultType::kUnableToCreateVectorNode)
    { }
};

struct UnableToCreateAttributeAccessNode : public ParserException
{
    UnableToCreateAttributeAccessNode(LexerToken token)
        : ParserException("Unable to create attribute access node", token,
                         ParserResultType::kUnableToCreateAttributeAccessNode)
    { }
};

struct ValueNodeWrongType : public ParserException
{
    ValueNodeWrongType(LexerToken token)
        : ParserException("Expecting string, number, vector or object name",
                         token, ParserResultType::kValueNodeWrongType)
    { }
};

struct UnexpectedEndOfVector : public ParserException
{
    UnexpectedEndOfVector(LexerToken token)
        : ParserException("Unexpected end of vector",
                         token, ParserResultType::kUnexpectedEndOfVector)
    { }
};

class Parser
{
public:
    /**
     * Parses the data from a file, containing the layer's markup language.
     * If the file does not exist or the parser is unable to open the file,
     * the parser will stop the process and return with an parser error result.
     *
     * @param file - Path to the file from where the data will be loaded.
     *               Absolute and relative paths are allowed.
     * @param dest - Reference to a GlobalObject receiving the generated
     *               object tree.
     * @return Result of the operation as a ParserResult, containing error type,
     *         message and line positions. If there was no error,
     *         the ParesrResultType will be set to ParserErrorType::kSuccess and
     *         the line positions are 0. Everything else is an error that
     *         occured while loading the file. This can contain an error from
     *         the Lexer, Parser and Interpreter. The parser will stop the
     *         process and the global_object will be empty when an error occured
     *         while parsing the file.
     */
    ParserResult ParseFile(const std::string file, GlobalObject& dest);

    /**
     * Parses raw string, containing the layer's markup language.
     *
     * @param data - String containg the layer's markup language.
     * @param dest - Reference to a GlobalObject receiving the generated
     *               object tree.
     * @return Result of the operation as a ParserResult, containing error type,
     *         message and line positions. If there was no error,
     *         the ParesrResultType will be set to ParserErrorType::kSuccess and
     *         the line positions are 0. Everything else is an error that
     *         occured while loading the file. This can contain an error from
     *         the Lexer, Parser and Interpreter. The parser will stop the
     *         process and the global_object will be empty when an error occured
     *         while parsing the file.
     */
    // NOTE: This function will be implemented later.
    //       Currently the focus is to be able to parse the data from a file.
    // ParserResult ParseData(const std::string data, GlobalObject& dest);

private:
    /**
     * Main lexer that is used to generate the tokens.
     */
    Lexer lexer_;

    /**
     * Resets the entire parser.
     */
    void Reset();

    /**
     * Processes the next token and generates new nodes based on the tokens.
     *
     * @param parent_node - Parent node of the new nodes that will be
     *                      generated.
     * @throws The function can throw parser and std exceptions.
     *         The parser will only catch the parser exceptions.
     *         Every other exceptions is not catched by the parser!
     */
    void ProcessTokens(std::shared_ptr<ParserNode> parent_node);

    /**
     * Checks if the current token is the end of a block.
     *
     * @param current_node - The current parent node. Used to check if the
     *                       block end is not in the global scope.
     * @throws UnexpectedEndOfBlock when the end of a block is in global scope
     */
    inline bool TokenIsBlockEnd(const ParserNode& current_node);

    /**
     * Checks if the current token is the start of an object.
     * The start of an object is defined by one of the following token sequence:
     *   - Data, Colon
     *   - Data, CBracketOpen
     */
    bool TokenIsObjectNode();
    void CreateObjectNode(ParserNode& parent_node);

    /**
     * Checks if the current token is the start of an attribute assign node.
     * The start of an object is defined by one of the following token sequence:
     *   - Data, Equals
     */
    bool TokenIsAttributeAssignNode();
    void CreateAttributeAssignNode(ParserNode& parent_node);

    /**
     * Checks if the current token is the start of a string node.
     * The start of an object is defined by one of the following token sequence:
     *   - String
     */
    bool TokenIsStringNode();
    std::shared_ptr<ParserStringNode> CreateStringNode();

    /**
     * Checks if the current token is the start of a number node.
     * The start of an object is defined by one of the following token sequence:
     *   - String
     */
    bool TokenIsNumberNode();
    std::shared_ptr<ParserNumberNode> CreateNumberNode();

    /**
     * Checks if the current token is the start of a vector node.
     * The start of an object is defined by one of the following token sequence:
     *   - String
     */
    bool TokenIsVectorNode();
    std::shared_ptr<ParserVectorNode> CreateVectorNode();

    /**
     * Checks if the current token is the start of an attribute access node.
     * The start of an object is defined by one of the following token sequence:
     *   - Data
     */
    bool TokenIsAttributeAccessNode();
    std::shared_ptr<ParserAttributeAccessNode> CreateAttributeAccessNode();
};

}  // namespace gui

#endif  // IMGUI_LAYER_INCLUDE_IMGUI_LAYER_PARSER_PARSER_H_
