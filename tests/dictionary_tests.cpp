#include <gtest/gtest.h>

#include <string>
#include <type_traits>
#include <utility>

#include "containers/BSTDictionary.hpp"
#include "containers/IDictionary.hpp"
#include "common/Exceptions.hpp"

TEST(KeyValuePairTests, ConstructorStoresKeyAndValue) {
    KeyValuePair<int, std::string> pair(5, "five");

    EXPECT_EQ(pair.key, 5);
    EXPECT_EQ(pair.value, "five");
}

TEST(KeyValuePairTests, ComparisonUsesOnlyKey) {
    KeyValuePair<int, std::string> first(3, "first");
    KeyValuePair<int, std::string> sameKey(3, "second");
    KeyValuePair<int, std::string> largerKey(4, "first");

    EXPECT_FALSE(first < sameKey);
    EXPECT_FALSE(sameKey < first);
    EXPECT_TRUE(first < largerKey);
}

TEST(DictionaryTests, NewDictionaryIsEmpty) {
    BSTDictionary<int, std::string> dictionary;

    EXPECT_EQ(dictionary.GetCount(), 0);
    EXPECT_FALSE(dictionary.ContainsKey(7));
}

TEST(DictionaryTests, AddAndGetValue) {
    BSTDictionary<int, std::string> dictionary;
    dictionary.Add(7, "seven");

    EXPECT_EQ(dictionary.GetCount(), 1);
    EXPECT_TRUE(dictionary.ContainsKey(7));
    EXPECT_EQ(dictionary.Get(7), "seven");
}

TEST(DictionaryTests, AddingExistingKeyReplacesValue) {
    BSTDictionary<int, std::string> dictionary;

    dictionary.Add(7, "old");
    dictionary.Add(7, "new");

    EXPECT_EQ(dictionary.GetCount(), 1);
    EXPECT_EQ(dictionary.Get(7), "new");
}

TEST(DictionaryTests, MissingGetThrows) {
    BSTDictionary<int, int> dictionary;

    EXPECT_THROW(dictionary.Get(999), IndexOutOfRangeException);
}

TEST(DictionaryTests, RemoveDeletesOnlyRequestedKey) {
    BSTDictionary<int, int> dictionary;

    dictionary.Add(1, 10);
    dictionary.Add(2, 20);
    dictionary.Remove(1);

    EXPECT_EQ(dictionary.GetCount(), 1);
    EXPECT_FALSE(dictionary.ContainsKey(1));
    EXPECT_EQ(dictionary.Get(2), 20);
}

TEST(DictionaryTests, RemovingMissingKeyThrows) {
    BSTDictionary<int, int> dictionary;

    EXPECT_THROW(dictionary.Remove(999), IndexOutOfRangeException);
}

TEST(DictionaryTests, TryGetWritesFoundValue) {
    BSTDictionary<int, int> dictionary;
    dictionary.Add(5, 123);

    int value = -1;

    EXPECT_TRUE(dictionary.TryGet(5, value));
    EXPECT_EQ(value, 123);
}

TEST(DictionaryTests, TryGetLeavesOutputUnchangedWhenMissing) {
    BSTDictionary<int, int> dictionary;
    int value = 777;

    EXPECT_FALSE(dictionary.TryGet(5, value));
    EXPECT_EQ(value, 777);
}

TEST(DictionaryTests, TryGetWorksOnConstDictionary) {
    BSTDictionary<int, int> dictionary;
    dictionary.Add(5, 123);

    const auto& constDictionary = dictionary;
    int value = 0;

    EXPECT_TRUE(constDictionary.TryGet(5, value));
    EXPECT_EQ(value, 123);
}

TEST(DictionaryTests, WorksThroughInterface) {
    static_assert(
        std::is_abstract<IDictionary<int, int>>::value,
        "IDictionary must be abstract"
    );

    BSTDictionary<int, int> dictionary;
    IDictionary<int, int>& interface = dictionary;

    interface.Add(3, 30);

    EXPECT_TRUE(interface.ContainsKey(3));
    EXPECT_EQ(interface.Get(3), 30);
    EXPECT_EQ(interface.GetCount(), 1);
    EXPECT_GE(interface.GetCapacity(), interface.GetCount());

    interface.Remove(3);

    EXPECT_EQ(interface.GetCount(), 0);
}

TEST(DictionaryTests, GetEntriesContainsAllPairs) {
    BSTDictionary<int, int> dictionary;
    dictionary.Add(3, 30);
    dictionary.Add(1, 10);
    dictionary.Add(2, 20);

    auto entries = dictionary.GetEntries();

    ASSERT_EQ(entries.GetLength(), 3);

    int foundOne = 0;
    int foundTwo = 0;
    int foundThree = 0;

    auto* it = entries.GetEnumerator();

    while (it->MoveNext()) {
        auto entry = std::as_const(*it).GetCurrent();

        switch (entry.key) {
        case 1:
            ++foundOne;
            EXPECT_EQ(entry.value, 10);
            break;

        case 2:
            ++foundTwo;
            EXPECT_EQ(entry.value, 20);
            break;

        case 3:
            ++foundThree;
            EXPECT_EQ(entry.value, 30);
            break;

        default:
            ADD_FAILURE() << "Unexpected key: " << entry.key;
            break;
        }
    }

    delete it;

    EXPECT_EQ(foundOne, 1);
    EXPECT_EQ(foundTwo, 1);
    EXPECT_EQ(foundThree, 1);
}

TEST(DictionaryTests, CopyIsIndependent) {
    BSTDictionary<int, int> original;
    original.Add(1, 10);

    BSTDictionary<int, int> copy = original;
    copy.Add(1, 99);
    copy.Add(2, 20);

    EXPECT_EQ(original.GetCount(), 1);
    EXPECT_EQ(original.Get(1), 10);
    EXPECT_FALSE(original.ContainsKey(2));
}