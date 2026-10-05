#include "aiws/text_processor.hpp"

#include <stdexcept>

namespace aiws {

std::vector<TokenInfo> TextProcessor::tokenize(const std::string& text) {

    // TODO: produce normalized tokens with source and paragraph information.

    std::vector<TokenInfo> tokens;

    std::size_t i = 0;

    std::size_t paragraph = 0;

    bool newline = false;

    while (i < text.size()) {

        unsigned char c = static_cast<unsigned char>(text[i]);

        // skips separators

        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9'))) {

            if (c == '\n') {

                if (newline) {

                    paragraph++;

                }

                newline = true;

            }

            else if (c != '\r' && c != ' ' && c != '\t') {

                newline = false;

            }

            i++;

            continue;

        }

        // Begining of a token

        std::size_t begin = i;

        std::string word;

        while (i < text.size()) {

            c = static_cast<unsigned char>(text[i]);

            if (!((c >= 'A' && c <= 'Z') ||
                  (c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9'))) {

                break;

            }

            if (c >= 'A' && c <= 'Z') {

                c = c - 'A' + 'a';

            }

            word.push_back(static_cast<char>(c));

            i++;

        }

        tokens.push_back(TokenInfo{word, begin, i, paragraph});

        newline = false;

    }

    return tokens;

}



std::vector<std::string> TextProcessor::terms(const std::string& text) {

    // TODO: return the normalized terms represented by the input text.

    std::vector<TokenInfo> tokens = tokenize(text);

    std::vector<std::string> words;

    for (const auto& token : tokens) {

        words.push_back(token.token);

    }

    return words;

}



std::string TextProcessor::normalize(const std::string& text) {

    // TODO: return the normalized form of the input text

    std::vector<TokenInfo> tokens = tokenize(text);

    return join(tokens, 0, tokens.size());

}



std::string TextProcessor::join(const std::vector<TokenInfo>& tokens,

                                std::size_t begin,

                                std::size_t end) {

    // TODO: join the requested token range into normalized text.

    if (begin > end || end > tokens.size()) {

        throw std::out_of_range("invalid token range");

    }

    std::string result;

    for (std::size_t i = begin; i < end; i++) {

        if (!result.empty()) {

            result += " ";

        }

        result += tokens[i].token;

    }

    return result;

}



std::string TextProcessor::join(const std::vector<std::string>& tokens,

                                std::size_t begin,

                                std::size_t end) {

    // TODO: join the requested term range into normalized text.

    if (begin > end || end > tokens.size()) {

        throw std::out_of_range("invalid token range");

    }

    std::string result;

    for (std::size_t i = begin; i < end; i++) {

        if (!result.empty()) {

            result += " ";

        }

        result += tokens[i];

    }

    return result;

}

} // namespace aiws