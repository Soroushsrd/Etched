#include "Tokenizer.h"
#include <gtest/gtest.h>

TEST(TokenizerTest, SimpleArrow) {
    Tokenizer t("a -> b");
    auto result = t.Tokenize();
    ASSERT_TRUE(result.isOk());

    auto &tokens = result.value();
    // a, ->, b, END
    ASSERT_EQ(tokens.size(), 4);
    EXPECT_EQ(tokens[0].getType(), TokenTag::IDENTIFIER);
    EXPECT_EQ(tokens[1].getType(), TokenTag::ARROW);
    EXPECT_EQ(tokens[2].getType(), TokenTag::IDENTIFIER);
    EXPECT_EQ(tokens[3].getType(), TokenTag::END);
}

TEST(TokenizerTest, UnterminatedString) {
    Tokenizer t("\"hello");
    auto result = t.Tokenize();
    ASSERT_TRUE(result.isErr());
    EXPECT_EQ(result.error().phase, "Tokenizer");
}
