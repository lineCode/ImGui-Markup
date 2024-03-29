#ifndef IMGUI_MARKUP_INCLUDE_IMGUI_MARKUP_PARSER_PARSER_H_
#define IMGUI_MARKUP_INCLUDE_IMGUI_MARKUP_PARSER_PARSER_H_

#include "imgui_markup/parser/parser_result.h"
#include "imgui_markup/objects/global_object.h"
#include "imgui_markup/parser/lexer.h"
#include "imgui_markup/parser/interpreter.h"
#include "imgui_markup/parser/parser_nodes.h"

#include <string>
#include <vector>

namespace imgui_markup::internal::parser
{

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

struct UnableToCreateBoolNode : public ParserException
{
    UnableToCreateBoolNode(LexerToken token)
        : ParserException("Unable to create bool node", token,
                         ParserResultType::kUnableToCreateBoolNode)
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

struct MissingVectorValue : public ParserException
{
    MissingVectorValue(LexerToken token)
        : ParserException("Missing value after ','",
                         token, ParserResultType::kMissingVectorValue)
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
     * Main interpreter to convet the node tree to the final object tree.
     */
    Interpreter interpreter_;

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
     * The start of an assign node is defined by one of the following
     * token sequence:
     *   - Data, Equals
     */
    bool TokenIsAttributeAssignNode();
    void CreateAttributeAssignNode(ParserNode& parent_node);

    /**
     * Checks if the current token is the start of a string node.
     * The start of a string node is defined by one of the following
     * token sequence:
     *   - String
     */
    bool TokenIsStringNode();
    std::shared_ptr<ParserStringNode> CreateStringNode();

    /**
     * Checks if the current token is the start of a int node.
     * The start of a int node is defined by one of the following
     * token sequence:
     *   - Int
     */
    bool TokenIsIntNode();
    std::shared_ptr<ParserIntNode> CreateIntNode();

    /**
     * Checks if the current token is the start of a float node.
     * The start of a float node is defined by one of the following
     * token sequence:
     *   - Float
     */
    bool TokenIsFloatNode();
    std::shared_ptr<ParserFloatNode> CreateFloatNode();

    /**
     * Checks if the current token is the start of a bool node.
     * The start of a boo node is defined by one of the following
     * token sequence:
     *   - Bool
     */
    bool TokenIsBoolNode();
    std::shared_ptr<ParserBoolNode> CreateBoolNode();

    /**
     * Checks if the current token is the start of a vector node.
     * The start of a vector node is defined by one of the following
     * token sequence:
     *   - BracketOpen
     */
    bool TokenIsVectorNode();
    std::shared_ptr<ParserVectorNode> CreateVectorNode();

    /**
     * Checks if the current token is the start of an attribute access node.
     * The start of a attribute access node is defined by one of the following
     * token sequence:
     *   - Data
     */
    bool TokenIsAttributeAccessNode();
    std::shared_ptr<ParserAttributeAccessNode> CreateAttributeAccessNode();
};

}  // namespace imgui_markup::internal::parser

#endif  // IMGUI_MARKUP_INCLUDE_IMGUI_MARKUP_PARSER_PARSER_H_
