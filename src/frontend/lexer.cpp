#include "frontend/lexer.h"
#include <cctype>

// 注册 Xaq 的关键字
const std::unordered_map<std::string, TokenType> Lexer::keywords_ = { { "auto", TokenType::AUTO },
    { "func", TokenType::FUNC }, { "return", TokenType::RETURN }, { "if", TokenType::IF }, { "else", TokenType::ELSE },
    { "for", TokenType::FOR }, { "in", TokenType::IN }, { "class", TokenType::CLASS }, { "method", TokenType::METHOD },
    { "operator", TokenType::OPERATOR }, { "this", TokenType::THIS }, { "Private", TokenType::PRIVATE },
    { "import", TokenType::IMPORT }, { "as", TokenType::AS }, { "true", TokenType::TRUE_LITERAL },
    { "false", TokenType::FALSE_LITERAL }, { "null", TokenType::NULL_LITERAL } };

Lexer::Lexer(std::string source)
    : source_(std::move(source)) { }

// 核心循环：只要没到文件末尾，就一直扫描
std::vector<Token> Lexer::scan_tokens() {
    while (!is_at_end()) {
        start_ = current_; // 记录下一个单词的起始位置
        scan_token();
    }
    tokens_.emplace_back(TokenType::END_OF_FILE, "", line_);
    return tokens_;
}

// 基础辅助方法
bool Lexer::is_at_end() const { return current_ >= source_.length(); }
char Lexer::advance() { return source_[current_++]; }
char Lexer::peek() const { return is_at_end() ? '\0' : source_[current_]; }
char Lexer::peek_next() const { return current_ + 1 >= source_.length() ? '\0' : source_[current_ + 1]; }
bool Lexer::match(char expected) {
    if (is_at_end() || source_[current_] != expected)
        return false;
    current_++;
    return true;
}
// 用于向结果列表添加 Token
void Lexer::add_token(TokenType type) {
    std::string text = source_.substr(start_, current_ - start_);
    tokens_.emplace_back(type, text, line_);
}

void Lexer::scan_token() {
    char c = advance();
    switch (c) {
    // 单字符符号
    case '(':
        add_token(TokenType::LEFT_PAREN);
        break;
    case ')':
        add_token(TokenType::RIGHT_PAREN);
        break;
    case '{':
        add_token(TokenType::LEFT_BRACE);
        break;
    case '}':
        add_token(TokenType::RIGHT_BRACE);
        break;
    case '[':
        add_token(TokenType::LEFT_BRACKET);
        break;
    case ']':
        add_token(TokenType::RIGHT_BRACKET);
        break;
    case ',':
        add_token(TokenType::COMMA);
        break;
    case '.':
        add_token(TokenType::DOT);
        break;
    case ';':
        add_token(TokenType::SEMICOLON);
        break;
    case '?':
        add_token(TokenType::QUESTION);
        break;

    // 双字符符号
    case '!':
        add_token(match('=') ? TokenType::BANG_EQUAL : TokenType::NOT);
        break;
    case '=':
        add_token(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
        break;
    case '-':
        if (match('>'))
            add_token(TokenType::ARROW); // 处理函数返回符 ->
        else if (match('='))
            add_token(TokenType::MINUS_EQUAL);
        else
            add_token(TokenType::MINUS);
        break;
    case '+':
        if (match('='))
            add_token(TokenType::PLUS_EQUAL);
        else
            add_token(TokenType::PLUS);
        break;
    case '<':
        if (match('='))
            add_token(TokenType::LESS_EQUAL);
        else if (match('<')) {
            if (match('='))
                add_token(TokenType::LESS_LESS_EQUAL);
            else
                add_token(TokenType::LESS_LESS);
        } else
            add_token(TokenType::LESS);
        break;
    case '>':
        if (match('='))
            add_token(TokenType::GREATER_EQUAL);
        else if (match('>')) {
            if (match('='))
                add_token(TokenType::GREATER_GREATER_EQUAL);
            else
                add_token(TokenType::GREATER_GREATER);
        } else
            add_token(TokenType::GREATER);
        break;
    case '%':
        if (match('='))
            add_token(TokenType::PERCENT_EQUAL);
        else
            add_token(TokenType::PERCENT);
        break;
    case '&':
        if (match('&'))
            add_token(TokenType::AMPERSAND_AMPERSAND);
        else
            add_token(TokenType::AMPERSAND);
        break;
    case '|':
        if (match('|'))
            add_token(TokenType::PIPE_PIPE);
        else
            add_token(TokenType::PIPE);
        break;
    case ':':
        if (match('='))
            add_token(TokenType::COLON_EQUAL); // :=
        else
            add_token(TokenType::COLON); // :
        break;
    // 三字符符号
    case '*':
        if (match('*')) {
            if (match('='))
                add_token(TokenType::STAR_STAR_EQUAL); // **=
            else
                add_token(TokenType::STAR_STAR); // **
        } else if (match('=')) {
            add_token(TokenType::STAR_EQUAL); // *=
        } else {
            add_token(TokenType::STAR); // *
        }
        break;

    // 处理斜杠（除法、单行注释、多行注释）
    case '/':
        if (match('/')) {
            // 单行注释：一直读取直到遇到换行符
            while (peek() != '\n' && !is_at_end())
                advance();
        } else if (match('*')) {
            // 多行注释 /* ... */
            block_comment();
        } else if (match('=')) {
            add_token(TokenType::SLASH_EQUAL); // /=
        } else {
            add_token(TokenType::SLASH);
        }
        break;

    // 处理字符串和文档字符串
    case '"':
        // 检查是不是 """ 文档字符串
        if (peek() == '"' && peek_next() == '"') {
            advance();
            advance(); // 吞掉后面两个引号
            doc_string(); // 解析多行字符串
        } else {
            string_literal(); // 解析普通字符串
        }
        break;

    // 忽略空白字符
    case ' ':
    case '\r':
    case '\t':
        break;
    case '\n':
        line_++;
        break;

    default:
        if (isdigit(c)) {
            number_literal();
        } else if (isalpha(c) || c == '_') {
            identifier();
        } else {
            std::cerr << "[Line " << line_ << "] Error: Unexpected character." << std::endl;
        }
        break;
    }
}

void Lexer::block_comment() {
    while (!is_at_end()) {
        if (peek() == '\n')
            line_++;

        // 找到了结束标志 */
        if (peek() == '*' && peek_next() == '/') {
            advance(); // 吞掉 *
            advance(); // 吞掉 /
            return;
        }
        advance();
    }
    std::cerr << "[Line " << line_ << "] Error: Unterminated block comment." << std::endl;
}

void Lexer::number_literal() {
    while (isdigit(peek()))
        advance();

    // 处理小数部分
    if (peek() == '.' && isdigit(peek_next())) {
        advance(); // 吞掉 '.'
        while (isdigit(peek()))
            advance();
    }

    add_token(TokenType::NUMBER);
}

void Lexer::identifier() {
    while (isalnum(peek()) || peek() == '_')
        advance();

    std::string text = source_.substr(start_, current_ - start_);
    auto keyword_it = keywords_.find(text);
    if (keyword_it != keywords_.end()) {
        add_token(keyword_it->second); // 关键字
    } else {
        add_token(TokenType::IDENTIFIER); // 普通标识符
    }
}

void Lexer::string_literal() {
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n')
            line_++;
        advance();
    }

    if (is_at_end()) {
        std::cerr << "[Line " << line_ << "] Error: Unterminated string." << std::endl;
        return;
    }

    advance(); // 吞掉结尾的引号
    std::string value = source_.substr(start_ + 1, current_ - start_ - 2);
    tokens_.emplace_back(TokenType::STRING, value, line_);
}

void Lexer::doc_string() {
    while (!(peek() == '"' && peek_next() == '"' && (current_ + 2 < source_.length() && source_[current_ + 2] == '"'))
        && !is_at_end()) {
        if (peek() == '\n')
            line_++;
        advance();
    }

    if (is_at_end()) {
        std::cerr << "[Line " << line_ << "] Error: Unterminated doc string." << std::endl;
        return;
    }

    advance(); // 吞掉第一个引号
    advance(); // 吞掉第二个引号
    advance(); // 吞掉第三个引号
    add_token(TokenType::STRING); // 文档字符串也当作 STRING 处理
}