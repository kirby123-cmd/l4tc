#include "./tokenizer.hpp"

namespace tokenizer {
  Token *create_next_token_sub(char *p, int &line, bool is_indent) {
    assert(line);
    if (!*p) return NULL;
    if ('0' <= *p && *p <= '9') {
      int length = 0;
      while ('0' <= p[length] && p[length] <= '9') length++;
      return new Token(line, std::string_view(p).substr(0, length), NumberConstant);
    }
    if (('A' <= *p && *p <= 'Z') || ('a' <= *p && *p <= 'z') || *p == '_') {
      int length = 0;
      while (('A' <= p[length] && p[length] <= 'Z') ||
            ('a' <= p[length] && p[length] <= 'z') || p[length] == '_' ||
            ('0' <= p[length] && p[length] <= '9')) length++;
      Token *ret = new Token(line, std::string_view(p).substr(0, length), Ident);
      if (ret->sv == "break") ret->type = KwBreak;            // break
      else if (ret->sv == "continue") ret->type = KwContinue; // continue
      else if (ret->sv == "elif") ret->type = KwElif;         // elif
      else if (ret->sv == "else") ret->type = KwElse;         // else
      else if (ret->sv == "func") ret->type = KwFunc;         // func
      else if (ret->sv == "if") ret->type = KwIf;             // if
      else if (ret->sv == "loop") ret->type = KwLoop;         // loop
      else if (ret->sv == "num") ret->type = KwNum;           // num
      else if (ret->sv == "return") ret->type = KwReturn;     // return
      else if (ret->sv == "str") ret->type = KwStr;           // str
      else if (ret->sv == "void") ret->type = KwVoid;         // void
      return ret;
    }
    if ('!' == *p && p[1] == '=') {
      return new Token(line, std::string_view(p).substr(0, 2), Punctuator);                         // !=
    }
    if ('&' == *p) {
      if (p[1] == '&') return new Token(line, std::string_view(p).substr(0, 2), Punctuator);        // &&
      return new Token(line, std::string_view(p).substr(0, 1), Punctuator);                         // &
    }
    if ('|' == *p) {
      if (p[1] == '|') return new Token(line, std::string_view(p).substr(0, 2), Punctuator);        // ||
      return new Token(line, std::string_view(p).substr(0, 2), Punctuator);                         // |
    }
    if ('<' == *p) {
      if (p[1] == '<') return new Token(line, std::string_view(p).substr(0, 2), Punctuator);        // <<
      if (p[1] == '=') return new Token(line, std::string_view(p).substr(0, 2), Punctuator);        // <=
      return new Token(line, std::string_view(p).substr(0, 1), Punctuator);                         // <
    }
    if ('>' == *p) {
      if (p[1] == '>') return new Token(line, std::string_view(p).substr(0, 2), Punctuator);        // >>
      if (p[1] == '=') return new Token(line, std::string_view(p).substr(0, 2), Punctuator);        // >=
      return new Token(line, std::string_view(p).substr(0, 1), Punctuator);                         // >
    }
    if ('-' == *p) {
      if (p[1] == '>') return new Token(line, std::string_view(p).substr(0, 2), Punctuator);        // ->
      return new Token(line, std::string_view(p).substr(0, 1), Punctuator);                         // -
    }
    if ('\n' == *p) {
      line++;
      if ('\n' == p[1]) return new Token(line, std::string_view(p).substr(0, 1), Delimiter);
      return new Token(line, std::string_view(p).substr(0, 1), Punctuator);
    }
    if (' ' == *p) {
      if (!is_indent) return new Token(line, std::string_view(p).substr(0, 1), Delimiter);
      int length = 0;
      while (' ' == p[length]) {
        if (' ' == p[++length]) length++;
        else return new Token(line, std::string_view(p).substr(0, length), Unknown);
      }
      return new Token(line, std::string_view(p).substr(0, length), Punctuator);
    }
    if (':' == *p || '=' == *p ||
        '^' == *p || '~' == *p ||
        '+' == *p ||
        '/' == *p || '*' == *p || '%' == *p ||
        '(' == *p || ')' == *p ||
        '[' == *p || ']' == *p ||
        ',' == *p) {
      return new Token(line, std::string_view(p).substr(0, 1), Punctuator);
    }
    return new Token(line, std::string_view(p).substr(0, 1), Unknown);
  }

  Token *create_next_token(char *p, int &line) {
    static bool is_indent = true;
    Token *ret = create_next_token_sub(p, line, is_indent);
    is_indent = ret != NULL &&
                ret->sv == "\n" && ret->type == Punctuator;
    return ret;
  }

  void print_tokens(Token *head) {
    for (Token *next = head; next != NULL; next = next->next) {
      if (next->sv.front() == ' ')
        fprintf(stderr, "%ld spaces ", next->sv.length());
      else if (next->sv == "\n")
        fprintf(stderr, "code: LF ");
      else
        fprintf(stderr, "code: aaaaaaa ");
        // fprintf(stderr, "code: %.*s ", next->length, next->begin);
      switch (next->type)
      {
        case Delimiter:
          fprintf(stderr, "type: Delimiter\n");
          break;
        case Punctuator:
          fprintf(stderr, "type: Punctuator\n");
          break;
        case NumberConstant:
          fprintf(stderr, "type: NumberConstant\n");
          break;
        case StringLiteral:
          fprintf(stderr, "type: StringLiteral\n");
          break;
        case Ident:
          fprintf(stderr, "type: Ident\n");
          break;
        case KwBreak:
          fprintf(stderr, "type: KwBreak\n");
          break;
        case KwContinue:
          fprintf(stderr, "type: KwContinue\n");
          break;
        case KwElif:
          fprintf(stderr, "type: KwElif\n");
          break;
        case KwElse:
          fprintf(stderr, "type: KwElse\n");
          break;
        case KwFunc:
          fprintf(stderr, "type: KwFunc\n");
          break;
        case KwIf:
          fprintf(stderr, "type: KwIf\n");
          break;
        case KwLoop:
          fprintf(stderr, "type: KwLoop\n");
          break;
        case KwNum:
          fprintf(stderr, "type: KwNum\n");
          break;
        case KwReturn:
          fprintf(stderr, "type: KwReturn\n");
          break;
        case KwStr:
          fprintf(stderr, "type: KwStr\n");
          break;
        case KwVoid:
          fprintf(stderr, "type: KwVoid\n");
          break;
        case Unknown:
          fprintf(stderr, "type: Unknown\n");
          break;
        default:
          break;
      }
    }
  }

  Token *tokenize(std::string &source) {
    Token *head = NULL;
    Token **next = &head;
    int l = 1;
    char *p = &source[0];
    for (Token *t=create_next_token(p, l);t!=NULL;) {
      *next = t;
      next = &t->next;
      p += t->sv.length();
      t = create_next_token(p, l);
    }
    return head;
  }
}