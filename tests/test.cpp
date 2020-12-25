// Copyright 2020 Your Name <your_email>

#include <gtest/gtest.h>

#include <hash.hpp>

TEST(Hash, check_hash) {
  EXPECT_TRUE(true);
  Hash hash1(
      "2807667562");  // hash:
                      // d3b2bbb81ac4c0f89d7c7ab4cca905f6c28180b7d96bca05c973149893040000
  Hash hash2(
      "550090725");  // hash:
                     // 50ecb11e33218167d63788e897eb1510573b0bb0f12b1403e1801859d081bd81
  EXPECT_TRUE(hash1.check());
  EXPECT_FALSE(hash2.check());
}

TEST(Hash, to_file) {
  Hash hash("2807667562");
  hash.set_time(1234567);
  std::vector<Hash> vec = {hash};
  std::ostringstream out;
  write_to_file(out, vec);
  EXPECT_EQ(out.str(),
            "[\n"
            "    {\n"
            "        \"data\": \"2807667562\",\n"
            "        \"hash\": "
            "\"d3b2bbb81ac4c0f89d7c7ab4cca905f6c28180b7d96bca05c973149893040000"
            "\",\n"
            "        \"timestamp\": 1234567\n"
            "    }\n"
            "]");
}
