#ifndef LIBUNDO_H
#define LIBUNDO_H

#include <diff_match_patch.h>

#include <cereal/access.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

/**
 * @brief      { struct_description }
 */
struct Node {
  int id;

  std::shared_ptr<Node> parent;
  std::vector<std::shared_ptr<Node>> children;

  std::pair<std::string, std::string> patches;
  std::chrono::time_point<std::chrono::system_clock> timestamp;

  template <class Archive>
  void serialize(Archive& ar) {
    ar(id, parent, patches, timestamp);
  }
};

class UndoTree {
 public:
  /**
   * @brief      { function_description }
   */
  UndoTree() : root(NULL), total(0), branch_idx(0), idx(0) {}
  ~UndoTree() {}

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
      root->patches = patch("", buf);
      root->parent = NULL;
    } else {
      std::shared_ptr<Node> parent = find_parent();
      parent->children.push_back(to_add);
      to_add->parent = parent;
      to_add->patches = patch(cur_buf, buf);
    }

    idx = to_add->id;
    cur_buf = buf;
  }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  std::string undo() {
    if (idx - 1 >= 0) {
      std::string patch = current_node()->patches.second;
      std::pair<std::string, std::vector<bool>> out =
          dmp.patch_apply(dmp.patch_fromText(patch), cur_buf);
      idx = idx - 1;
      return out.first;
    } else {
      return cur_buf;
    }
  }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  std::string redo() {
    if (idx + 1 <= total) {
      std::string patch = current_node()->patches.first;
      std::pair<std::string, std::vector<bool>> out =
          dmp.patch_apply(dmp.patch_fromText(patch), cur_buf);
      idx = idx + 1;
      return out.first;
    } else {
      return cur_buf;
    }
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
  int branch() { return branch_idx; }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  std::shared_ptr<Node> current_node() { return search(root, idx); }

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
    std::shared_ptr<Node> pos = current_node();
    if (branch_idx + 1 < pos->parent->children.size()) {
      branch_idx = branch_idx + 1;
    } else {
      branch_idx = 0;
    }
  }

 private:
  std::shared_ptr<Node> root;

  int total;
  int idx;
  int branch_idx;

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
    ar(root, total, idx, branch_idx, cur_buf, undo_file);
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
  std::shared_ptr<Node> search(std::shared_ptr<Node> root, int to_find) {
    if (!root || root->id == to_find) {
      return root;
    }

    for (std::shared_ptr<Node> child : root->children) {
      std::shared_ptr<Node> found = search(child, to_find);
      if (found) {
        return found;
      }
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
    if (maybe->parent && branch_idx < maybe->parent->children.size()) {
      return maybe->parent->children[branch_idx];
    } else {
      return maybe;
    }
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
    auto d2 = d1;

    for (auto& e : d2) {
      if (e.operation == DELETE) {
        e.operation = INSERT;
      } else if (e.operation == INSERT) {
        e.operation = DELETE;
      }
    }

    std::string p1 = dmp.patch_toText(dmp.patch_make(s1, d1));
    std::string p2 = dmp.patch_toText(dmp.patch_make(s2, d2));

    return std::make_pair(p1, p2);
  }
};

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UndoTree UndoTree;

UndoTree* new_tree();
UndoTree* load_tree(const char* path);
void save_tree(UndoTree* t, const char* path);
void insert(UndoTree* t, const char* buf);

#ifdef __cplusplus
}
#endif

#endif  // LIBUNDO_H
