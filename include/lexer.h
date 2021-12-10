#pragma once

#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <cstddef>

#include "errors.h"
#include "guide.h"

/**
 Precedence (prc) constants.
*/
const short g_lowest_prc = 0;
const short g_low_prc = 1;
const short g_high_prc = 2;
const short g_highest_prc = 3;

const char g_lexicon_delimiters[2] = {' ', ';'};

/**
 * mlnum       : Numeric type, every numeric string of characters
 * mlnamespace : Namespace type, every literal string of characters until it is resolved
 *               into it's real type
 * mlaffix     : Affix type, an exhaustible built-in or user defined type of a literal string of characters
 * mlprefix    : Prefix type, an exhustible built-in or user defined type of an affix that is found pre statement
 * mlinfix     : Infix type, an exhustible built-in or user defined type of an affix that is found in between a statement
 * mlcircumfix : Circumfix type, an exhustible built-in or user defined type of an affix that is found surrounding
 *               the statement at the start and the end
 * mleof       : EOF type, remarks the end of a token stream.
*/
enum class MlTypes
{
    mlnum,
    mlnamespace,
    mlaffix,
    mlprefix,
    mlinfix,
    mlcircumfix,
    mleof
};

std::string get_type_name(const MlTypes& tp);


struct Lexeme
{
    std::string characters;
    MlTypes typehint;
    short precedence;
    bool hyphenated;
    bool isolated;
    bool unary;
    bool binary;

    bool operator==(Lexeme &other);

    bool operator!=(Lexeme &other);
};

/**
  A Token is a simple container with a meaning (morpheme) and it's position in
  the source code.
*/
struct Token
{
    Lexeme lexeme;
    Position position;
    bool valid;

    Token() : valid(false) {};

    Token(Position position)
        : position(position), valid(false) {};

    Token(Lexeme means, Position pos)
        : lexeme(means), position(pos), valid(true) {};

    bool operator==(Token& other);

    bool operator!=(Token& other);

    std::string to_string() const;

    friend std::ostream& operator<<(std::ostream& os, const Token& n);
};

Lexeme mlNumInfix(int stack, const Lexeme& op, const Lexeme& operand);

Lexeme mlNumInfix(int stack, const Lexeme& op);

Token mlNumConclude(TypeGuide& guide);

Lexeme mlNamespaceInfix(int stack, const Lexeme& op, const Lexeme& operand);

Lexeme mlNamespaceInfix(int stack, const Lexeme& op);

Token mlNamespaceConConclude(TypeGuide& guide, std::string result, Position pos);

Token mlNamespaceConclude(TypeGuide& guide);


struct LexxedResult
{
    std::unique_ptr<BaseException> error;
    std::vector<Token> tokens;
    bool failed;

    LexxedResult(std::vector<Token>& tks)
        : tokens(tks), failed(false) {};

    LexxedResult(std::unique_ptr<BaseException>& fault)
        : error(std::move(fault)), failed(true) {}
};


LexxedResult tokenize();

const Lexeme g_lexicon_affixes[12] =
{ {"~", MlTypes::mlinfix, g_lowest_prc, false},
  {"=", MlTypes::mlinfix, g_lowest_prc, false},
  {"+", MlTypes::mlinfix, g_lowest_prc, false},
  {"-", MlTypes::mlinfix, g_lowest_prc, false},
  {"*", MlTypes::mlinfix, g_low_prc, false },
  {"/", MlTypes::mlinfix, g_low_prc, false},
  {"...", MlTypes::mlaffix, g_low_prc, false},
  {"return", MlTypes::mlprefix, g_low_prc, false, true},
  {"{", MlTypes::mlcircumfix, g_lowest_prc},
  {"}", MlTypes::mlcircumfix, g_lowest_prc},
  {"(", MlTypes::mlcircumfix, g_lowest_prc},
  {")", MlTypes::mlcircumfix, g_lowest_prc} };