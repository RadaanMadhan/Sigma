#pragma once

#include <string>
#include <vector>


enum class TokenType {
  exit, int_lit, semi, open_paren, close_paren, ident, let, eq, plus, multi, minus, div
};

bool isBinaryOperator(TokenType t) {
    switch (t) {
      case TokenType::plus:
      case TokenType::multi:
      case TokenType::minus:
      case TokenType::div:
        return true;
      default: return false;
    }
};

std::optional<int> get_precedence(TokenType t) {
  switch (t) {
    case TokenType::plus:
      return 0;
    case TokenType::minus:
      return 1;
    case TokenType::multi:
      return 2;
    case TokenType::div:
      return 3;
    default:
      return {};
  }
};

struct Token {
  TokenType type;
  std::optional<std::string> value{};
};

class Tokenizer {
public:
  inline explicit Tokenizer(std::string src)
    :m_src(std::move(src))
  {
  }

  inline std::vector<Token> tokenize() {
    std::string buffer;
    std::vector<Token> tokens;

    while(peek().has_value()) {
      if (std::isalpha(peek().value())) {
        buffer.push_back(consume());
        while (peek().has_value() && std::isalpha(peek().value())) {
          buffer.push_back(consume());
        }
        if (buffer == "exit") {
          tokens.push_back({.type = TokenType::exit});
          buffer.clear();
          continue;
        }
        else if (buffer == "let") {
          tokens.push_back({.type = TokenType::let});
          buffer.clear();
          continue;
        }
        else {
          tokens.push_back({.type = TokenType::ident, .value = buffer});
          buffer.clear();
          continue;
        }
      }
      else if (std::isdigit(peek().value())) {
        buffer.push_back(consume());
        while (peek().has_value() && std::isdigit(peek().value())) {
          buffer.push_back(consume());
        }
        tokens.push_back({.type = TokenType::int_lit, .value = buffer});
        buffer.clear();
        continue;
      }
      else if (peek().value()== '(') {
        consume();
        tokens.push_back({.type = TokenType::open_paren});
      }
      else if (peek().value()== ')') {
        consume();
        tokens.push_back({.type = TokenType::close_paren});
      }
      else if (peek().value() == ';') {
        consume();
        tokens.push_back({.type = TokenType::semi});
        //buffer.clear();
        continue;
      }
      else if (peek().value() == '=') {
        consume();
        tokens.push_back({.type = TokenType::eq});
        //buffer.clear();
        continue;
      }
      else if(peek().value() == '+') {
        tokens.push_back({.type = TokenType::plus});
        consume();
        continue;
      }
      else if(peek().value() == '*') {
        tokens.push_back({.type = TokenType::multi});
        consume();
        continue;
      }
      else if(peek().value() == '-') {
        tokens.push_back({.type = TokenType::minus});
        consume();
        continue;
      }
      else if(peek().value() == '/') {
        tokens.push_back({.type = TokenType::div});
        consume();
        continue;
      }
      else if (std::isspace(peek().value())) {
        consume();
        continue;
      }
      else {
        std::cerr << "womp womp" << std::endl;
        exit(EXIT_FAILURE);
      }
    }
    m_index = 0;
    return tokens;
  }


private:

  [[nodiscard]] std::optional<char> peek(int const offset = 0) const{
    if(m_index + offset >= m_src.size()) {
      return {};
    }
    else {
      return m_src.at(m_index + offset);
    }
  }

  char consume() {
      return m_src.at(m_index++);
  }

  const std::string m_src;
  size_t m_index = 0;
};