#include <stdio.h>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>

#include "lexer.h"
#include "task.h"

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Class:    CMyClass

  Summary:  Short summary of purpose and content of CMyClass.
            Short summary of purpose and content of CMyClass.

  Methods:  MyMethodOne
              Short description of MyMethodOne.
            MyMethodTwo
              Short description of MyMethodTwo.
            CMyClass
              Constructor.
            ~CMyClass
              Destructor.
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

void TypeGuide::advance()
{
    this->pos++;
    this->column++;

    if (this->newLine == true)
    {
        this->lastLineMax = this->column;
        this->line++;
        this->newLine = false;
    };
    this->updateChar();
    if (this->chr == '\n')
    {
        this->newLine = true;
    };
};

Position TypeGuide::capture()
{
    return Position(this->pos, this->pos, this->column, this->line);
};

void TypeGuide::retreat()
{
    this->pos--;
    this->column--;

    this->updateChar();
    if (this->chr == '\n')
    {
        this->column = (int)this->lastLineMax;
        this->line--;
    };
}

void TypeGuide::updateChar()
{
    if ((unsigned)this->pos < this->text.length())
    {
        this->chr = this->text[this->pos];
    }
    else
    {
        this->chr = '!';
        this->eof = true;
    }
};

bool TypeGuide::chrIsDelim()
{
    const char *uncover = std::find(std::begin(g_lexicon_delimiters),
                                    std::end(g_lexicon_delimiters), this->chr);
    return !(uncover == std::end(g_lexicon_delimiters));
}

std::string Position::to_string()
{
    return std::to_string(this->line) + ":" + std::to_string(this->column) + "~" + std::to_string(this->end);
};

bool Token::isValid()
{
    return this->valid;
};

bool Token::operator==(Token &other)
{
    return this->isValid() && other.isValid() && this->meaning == other.meaning;
};

bool Token::operator!=(Token &other)
{
    return !this->operator==(other);
};

std::string Token::to_string()
{
    return "Tk<" + this->meaning.to_string() + " @ " + this->position.to_string() + ">";
};

void Morpheme::setValue(std::string val)
{
    this->value = val;
};

void Morpheme::setTypehint(std::string val)
{
    this->typehint = val;
}

void Morpheme::setUnary(bool unary)
{
    this->unary = unary;
}

void Morpheme::setPrecedence(short prc)
{
    this->precedence = prc;
}

void Morpheme::setInterfix(bool interfix)
{
    this->interfix = interfix;
}

bool Morpheme::operator==(Morpheme &other)
{
    return this->value == other.value && this->typehint == other.typehint;
};

bool Morpheme::operator!=(Morpheme &other)
{
    return !this->operator==(other);
};

std::string Morpheme::to_string()
{
    return this->typehint + ":" + this->value;
};

//////////////////////////////////////////////////////////////
//               Types of Morphemes                         //
//////////////////////////////////////////////////////////////

struct MlNum : public Morpheme
{
    MlNum(){};

    MlNum(std::string val)
    {
        this->setValue(val);
        this->setTypehint(__func__);
    };

    MlNum(int val)
    {
        this->setValue(std::to_string(val));
        this->setTypehint(__func__);
    };

    Morpheme infix(const Morpheme &op, const Morpheme &operand, int stack) override
    {
        return MlNum();
    };

    Morpheme infix(const Morpheme &op, int stack) override
    {
        return MlNum();
    };

    static bool isNum(char key)
    {
        std::string candidates = "0123456789";
        int index = candidates.find(key);
        if (index == -1)
        {
            return false;
        }
        else
        {
            return true;
        }
    };

    static Token conclude(TypeGuide &guide)
    {
        std::string result;
        int dotCount = 0;
        int column = guide.column;
        int start = guide.pos;
        int line = guide.line;
        while (guide.eof != true && (MlNum::isNum(guide.chr) || guide.chr == '.') && guide.chrIsDelim() == false)
        {
            if (guide.chr == '.')
            {
                if (dotCount == 0)
                {
                    dotCount++;
                }
                else {
                    break;
                }
            }
            else
            {
                result += guide.chr;
                guide.advance();
            }
        };
        int end = guide.pos;
        return Token(MlNum(result), Position(start, end, column, line));
    };
};

struct MlString : public Morpheme
{
    static Token conclude(TypeGuide typeGuide) { return Token(); };
};

struct MlNamespace : public Morpheme
{
    MlNamespace(std::string val)
    {
        this->setValue(val);
        this->setTypehint(__func__);
    };

    static Token resolve(TypeGuide &guide, std::string result, Position pos)
    {
        while (guide.eof == false && guide.chrIsDelim() == false)
        {
            result += guide.chr;
            guide.advance();
        };
        pos.end = guide.pos;
        return Token(MlNamespace(result), pos);
    }

    static Token conclude(TypeGuide &guide, std::string result, Position pos)
    {
        return MlNamespace::resolve(guide, result, pos);
    };

    static Token conclude(TypeGuide &guide)
    {
        return MlNamespace::resolve(guide, "", guide.capture());
    };
};

struct MlAffix : public Morpheme
{
};

struct MlInfix : public Morpheme
{
    MlInfix(std::string val, bool isUnary, bool interfix, short precedence)
    {
        this->setValue(val);
        this->setTypehint(__func__);
        this->setPrecedence(precedence);
        this->setUnary(isUnary);
        this->setInterfix(interfix);
    };

    static Token conclude(TypeGuide &guide)
    {
        return Token();
    };
};

//////////////////////////////////////////////////////////////
//                   Naive Definition                       //
//////////////////////////////////////////////////////////////

const MlInfix g_lexicon_infixes[8] =
    {MlInfix("~", true, false, LOWEST),
     MlInfix("=", true, false, LOWEST),
     MlInfix("+", true, false, LOWER),
     MlInfix("-", true, false, LOWER),
     MlInfix("*", true, false, LOW),
     MlInfix("/", true, false, LOW),
     MlInfix("...", true, false, LOW),
     MlInfix("return", true, true, LOW)};

const MlInfix* isDefinedInfix(std::string chr)
{
    auto isEquals = [chr](MlInfix m)
    {
        return m.value == chr;
    };
    return std::find_if(std::begin(g_lexicon_infixes), std::end(g_lexicon_infixes), isEquals);
};

/**
 * Attempts to resolve a given character from the lexicon defined in the outset.
*/
Token MlResolve(TypeGuide &guide)
{
    std::string chr(1, guide.chr);
    Position capture = guide.capture();
    guide.advance();
    const MlInfix* res = isDefinedInfix(chr);
    if (res != std::end(g_lexicon_infixes))
    {
        capture.end = guide.pos;
        return Token(*res, capture);
    }

    auto startswith = [chr](Morpheme m)
    {
        return (int)m.value.rfind(chr) != -1;
    };
    // DEPRECATED
    // std::vector<Morpheme> matches;
    // std::copy_if(std::begin(g_lexicon_infixes), std::end(g_lexicon_infixes), std::back_inserter(matches), startswith);
    while (res == std::end(g_lexicon_infixes) && guide.eof == false && guide.chrIsDelim() == false)
    {
        chr += guide.chr;
        res = isDefinedInfix(chr);
        guide.advance();
    };
    capture.end = guide.pos;
    if (res != std::end(g_lexicon_infixes) && !((*res).interfix == true && (guide.chrIsDelim() == false && guide.eof == false)))
    {
        return Token(*res, capture);
    }
    else
    {
        /* Case where lexxed component is not predefined as an infix
           we pass it over to conclude a namespace */
        return MlNamespace::conclude(guide, chr, capture);
    };
};

//////////////////////////////////////////////////////////////
//                      Tokenizer                           //
//////////////////////////////////////////////////////////////

std::vector<Token> Lexer::tokenize(const std::string &text)
{
    std::vector<Token> tokens;
    TypeGuide typeGuide(text);

    typeGuide.advance();
    while (typeGuide.eof != true)
    {
        Token tk;
        /* We step over g_lexicon_delimiters under command of the lexer, however
           this does not mean we do the same in resolving any hinted characters. */
        if (typeGuide.chrIsDelim() == true)
        {
            typeGuide.advance();
            continue;
        };

        if (MlNum::isNum(typeGuide.chr) == true)
        {
            tk = MlNum::conclude(typeGuide);
        }
        else
        {
            /* Certain types exists as definitions in the lexicon so we'll check for them and if not
               it is defaulted the incoming characters are defaulted to a namespace. */
            tk = MlResolve(typeGuide);

            if (tk.isValid() == false)
            {
                tk = MlNamespace::conclude(typeGuide);
            };
        };
        if (tk.isValid() == true)
        {
            tokens.push_back(tk);
            /* Notice that if a token is valid, it will advance the typeguide safely onto
               the appended character so we'll have no need to do it. */
        }
        else
        {
            return tokens;
        }
    };
    // an EOF push_back
    tokens.push_back(Token(Morpheme()));

    return tokens;
};
