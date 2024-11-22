#pragma once

#include <vector>
#include <memory>
#include <stdexcept>
#include "Token.hpp"
#include "expr.hpp"
#include "TokenType.hpp"

namespace net::ancillarycat::loxograph {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

    std::shared_ptr<Expr> parse() {
        return expression();
    }

private:
    std::vector<Token> tokens;
    size_t current = 0;

    std::shared_ptr<Expr> expression() {
        return binary();
    }

    std::shared_ptr<Expr> binary() {
        auto expr = unary();

        while (match({TokenType::kPlus, TokenType::kMinus, TokenType::kStar, TokenType::kSlash,
                      TokenType::kEqualEqual, TokenType::kBangEqual, TokenType::kLess,
                      TokenType::kLessEqual, TokenType::kGreater, TokenType::kGreaterEqual})) {
            Token op = previous();
            auto right = unary();
            expr = std::make_shared<Binary>(op, expr, right);
        }

        return expr;
    }

    std::shared_ptr<Expr> unary() {
        if (match({TokenType::kBang, TokenType::kMinus})) {
            Token op = previous();
            auto right = unary();
            return std::make_shared<Unary>(op, right);
        }

        return primary();
    }

    std::shared_ptr<Expr> primary() {
        if (match({TokenType::kNumber, TokenType::kString})) {
            return std::make_shared<Literal>(previous());
        }

        if (match({TokenType::kTrue})) {
            return std::make_shared<Literal>(Token{TokenType::kTrue, "true"sv, {}, previous().line});
        }

        if (match({TokenType::kFalse})) {
            return std::make_shared<Literal>(Token{TokenType::kFalse, "false"sv, {}, previous().line});
        }

        if (match({TokenType::kNil})) {
            return std::make_shared<Literal>(Token{TokenType::kNil, "nil"sv, {}, previous().line});
        }

        if (match({TokenType::kLeftParen})) {
            auto expr = expression();
            consume(TokenType::kRightParen, "Expect ')' after expression.");
            return std::make_shared<Grouping>(expr);
        }

        // throw std::runtime_error("Unexpected token.");
    }

    bool match(const std::vector<TokenType>& types) {
        for (const auto& type : types) {
            if (check(type)) {
                advance();
                return true;
            }
        }
        return false;
    }

    bool check(TokenType type) const {
        if (is_at_end()) return false;
        return peek().type.type == type.type;
    }

    Token advance() {
        if (!is_at_end()) current++;
        return previous();
    }

    bool is_at_end() const {
        return peek().type.type == TokenType::kEndOfFile;
    }

    Token peek() const {
        return tokens[current];
    }

    Token previous() const {
        return tokens[current - 1];
    }

    Token consume(TokenType type, const std::string& message) {
        if (check(type)) return advance();
        // throw std::runtime_error(message);
    }
};

} // namespace net::ancillarycat::loxograph