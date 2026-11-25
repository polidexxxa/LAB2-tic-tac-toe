#ifndef _BST_DICTIONARY_HPP_
#define _BST_DICTIONARY_HPP_

#include "BinarySearchTree.hpp"
#include "common/Exceptions.hpp"
#include "ListSequence.hpp"
#include "IDictionary.hpp"
#include "common/Option.hpp"

template <typename TKey, typename TElement>
class KeyValuePair {
public: 
    TKey key;
    TElement value;

    KeyValuePair() = default;
    KeyValuePair(const TKey& k, const TElement& v) : key(k), value(v) {}
    explicit KeyValuePair(const TKey& k) : key(k), value(TElement()) {}

    bool operator<(const KeyValuePair& other) const {
        return key < other.key;
    }
};

template <typename TKey, typename TElement>
class BSTDictionary : public IDictionary<TKey, TElement> {
private:
    using Pair = KeyValuePair<TKey, TElement>;
    BinarySearchTree<Pair> tree;

public:
    BSTDictionary() = default;
    ~BSTDictionary() override = default;

    int GetCount() const override {
        return static_cast<int>(tree.GetSize());
    }

    int GetCapacity() const override {
        return GetCount();
    }

    bool ContainsKey(const TKey& key) const override {
        return tree.Contains(Pair(key));
    }

    void Add(const TKey& key, const TElement& element) override {
        Pair pair(key, element);
        if (tree.Contains(pair)) {
            tree.Remove(pair);
        }
        tree.Insert(pair);
    }

    TElement Get(const TKey& key) const override {
        TElement element{};

        if (TryGet(key, element)) {
            return element;
        }

        throw IndexOutOfRangeException(
            "Key not found in BSTDictionary"
        );
    }

    void Remove(const TKey& key) override {
        Pair pair(key);
        if (!tree.Remove(pair)) {
            throw IndexOutOfRangeException("Key not found for removal");
        }
    }

    ListSequence<Pair> GetEntries() const {
        ListSequence<Pair> entries;
        IEnumerator<Pair>* it = tree.GetEnumerator();
        while (it->MoveNext()) {
            entries.Append(it->GetCurrent());
        }
        delete it;
        return entries;
    }

    bool TryGet(const TKey& key, TElement& out_element) const {
        Option<Pair> found = tree.Find(Pair(key));

        if (found.IsNone()) {
            return false;
        }

        out_element = found.GetValue().value;
        return true;
    }
};

#endif // _BST_DICTIONARY_HPP_