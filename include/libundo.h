#ifndef LIBUNDO_H
#define LIBUNDO_H

#include <diff_match_patch.h>

#include <cereal/access.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace libundo {

/**
 * @brief      { struct_description }
 */
struct Node {
  int id;

  std::shared_ptr<Node> parent;

  std::vector<std::shared_ptr<Node>> children;
  std::map<int, std::string> patches;

  std::chrono::time_point<std::chrono::system_clock> timestamp;

  template <class Archive>
  void serialize(Archive& ar) {
    ar(id, parent, children, patches, timestamp);
  }
};

class UndoTree {
 public:
  /**
   * @brief      { function_description }
   */
  UndoTree() : root(NULL), total(0), b_idx(0), n_idx(0) {}
  ~UndoTree() {}

  void save(const std::string& path) {
    std::ofstream history(path, std::ios::out | std::ios::binary);
    if (history.is_open()) {
      cereal::BinaryOutputArchive archive(history);
      archive(*this);
      history.close();
    }
  }

  /**
   * @brief      { function_description }
   *
   * @param[in]  buf   The buffer
   *
   * @return     { description_of_the_return_value }
   */
  void insert(const std::string& buf) {
    std::shared_ptr<Node> to_add = std::make_shared<Node>();

    to_add->id = ++total;
    to_add->timestamp = std::chrono::system_clock::now();
    if (!root) {
      root = to_add;
      root->parent = NULL;
    } else {
      std::shared_ptr<Node> parent = find_parent();
      std::pair<std::string, std::string> patches = patch(cur_buf, buf);

      parent->children.push_back(to_add);
      parent->patches[total] = patches.first;

      to_add->parent = parent;
      to_add->patches[parent->id] = patches.second;
    }

    n_idx = to_add->id;
    cur_buf = buf;
    index[total] = to_add;
  }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  std::string undo() {
    std::shared_ptr<Node> parent = current_node()->parent;
    if (parent) {
      cur_buf = apply_patch(parent->id);
    }
    return cur_buf;
  }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  std::string redo() {
    std::shared_ptr<Node> n = current_node();
    if (n->children.size() > 0) {
      cur_buf = apply_patch(n->children[b_idx]->id);
    }
    return cur_buf;
  }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  int size() { return total; }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  int branch() { return b_idx; }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  std::shared_ptr<Node> current_node() { return search(n_idx); }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  std::string buffer() { return cur_buf; }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  std::vector<Node> nodes() { return collect(root.get()); }

  /**
   * @brief      { function_description }
   */
  void switch_branch(void) {
    if (b_idx + 1 < current_node()->children.size()) {
      b_idx = b_idx + 1;
    } else {
      b_idx = 0;
    }
  }

 private:
  std::shared_ptr<Node> root;
  std::map<int, std::shared_ptr<Node>> index;

  int total;
  int n_idx;
  int b_idx;

  std::string cur_buf;
  std::string undo_file;

  diff_match_patch<std::string> dmp;

  friend class cereal::access;
  /**
   * @brief      { function_description }
   *
   * @param      ar       The archive
   *
   * @tparam     Archive  { description }
   */
  template <class Archive>
  void serialize(Archive& ar) {
    ar(root, total, n_idx, b_idx, cur_buf, undo_file);
  }

  std::vector<Node> collect(Node* root) {
    std::vector<Node> collected;
    if (root != NULL) {
      collected.push_back(*root);
      for (auto child : root->children) {
        std::vector<Node> found = collect(child.get());
        collected.insert(collected.end(), found.begin(), found.end());
      }
    }
    return collected;
  }

  /**
   * @brief      Searches for the first match.
   *
   * @param      root     The root
   * @param[in]  to_find  To find
   *
   * @return     { description_of_the_return_value }
   */
  std::shared_ptr<Node> search(int to_find) {
    auto it = index.find(to_find);
    if (it != index.end()) {
      return it->second;
    }
    return NULL;
  }

  /**
   * @brief      { function_description }
   *
   * @param      root     The root
   * @param[in]  to_find  To find
   *
   * @return     { description_of_the_return_value }
   */
  std::shared_ptr<Node> find_parent() {
    std::shared_ptr<Node> maybe = current_node();
    if (maybe->parent && b_idx < maybe->parent->children.size()) {
      return maybe->parent->children[b_idx];
    } else {
      return maybe;
    }
  }

  /**
   * @brief      { function_description }
   *
   * @param[in]  id    The identifier
   *
   * @return     { description_of_the_return_value }
   */
  const std::string apply_patch(int id) {
    std::string patch = current_node()->patches[id];
    std::pair<std::string, std::vector<bool>> out =
        dmp.patch_apply(dmp.patch_fromText(patch), cur_buf);
    n_idx = id;
    return out.first;
  }

  /**
   * @brief      { function_description }
   *
   * @param[in]  s1    The s 1
   * @param[in]  s2    The s 2
   *
   * @return     { description_of_the_return_value }
   */
  std::pair<std::string, std::string> patch(std::string s1, std::string s2) {
    auto d1 = dmp.diff_main(s1, s2);
    std::string p1 = dmp.patch_toText(dmp.patch_make(s1, d1));

    for (auto& e : d1) {
      if (e.operation == DELETE) {
        e.operation = INSERT;
      } else if (e.operation == INSERT) {
        e.operation = DELETE;
      }
    }
    std::string p2 = dmp.patch_toText(dmp.patch_make(s2, d1));

    return std::make_pair(p1, p2);
  }
};

/**
 * Parse the given SGF file into a Collection of GameTrees.
 *
 * @param  input_file A path to an SGF file.
 * @return            A vector of GameTrees.
 */
inline void save(UndoTree* t, const std::string& path) {
  std::ofstream history(path, std::ios::out | std::ios::binary);
  if (history.is_open()) {
    cereal::BinaryOutputArchive archive(history);
    archive(*t);
    history.close();
  }
}

inline UndoTree* load(const std::string& path, const std::string& buf) {
  UndoTree* t = new UndoTree();

  std::ifstream history(path, std::ios::in | std::ios::binary);
  if (history.is_open()) {
    cereal::BinaryInputArchive archive(history);
    archive(*t);
    history.close();
  }

  if (t->size() > 0) {
    std::size_t old_hash = std::hash<std::string>{}(t->buffer());
    std::size_t new_hash = std::hash<std::string>{}(buf);

    if (old_hash != new_hash) {
      delete t;
      return reinterpret_cast<UndoTree*>(new UndoTree());
    }
  }

  return reinterpret_cast<UndoTree*>(t);
}
}

#endif  // LIBUNDO_H
