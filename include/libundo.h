#ifndef LIBUNDO_H
#define LIBUNDO_H

#include "diff_match_patch.h"

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
  int index;

  std::shared_ptr<Node> parent;
  std::vector<std::shared_ptr<Node>> children;

  std::pair<std::string, std::string> patches;
  std::chrono::time_point<std::chrono::system_clock> timestamp;
};

class UndoTree {
 public:
  /**
   * @brief      { function_description }
   *
   * @param[in]  hash  The hash
   * @param[in]  dir   The dir
   */
  UndoTree(const std::string& path) : root(NULL), total(0), branch(0), idx(0) {}

  /**
   * @brief      Destroys the object.
   */
  ~UndoTree(void) { write(root.get()); }

  /**
   * @brief      { function_description }
   *
   * @param[in]  buf   The buffer
   *
   * @return     { description_of_the_return_value }
   */
  void insert(const std::string& buf) {
    std::shared_ptr<Node> to_add = std::make_shared<Node>();
    to_add->index = ++total;

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

    idx = to_add->index;
    cur_buf = buf;
  }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  std::string undo() {
    if (idx - 1 >= 0) {
      std::string patch = current()->patches.second;
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
      std::string patch = current()->patches.first;
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
  int current_branch() { return branch; }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  std::shared_ptr<Node> current() { return search(root, idx); }

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
    std::shared_ptr<Node> pos = current();
    if (branch + 1 < pos->parent->children.size()) {
      branch = branch + 1;
    } else {
      branch = 0;
    }
  }

 private:
  std::shared_ptr<Node> root;

  int total;
  int idx;
  int branch;

  std::string cur_buf;
  std::string undo_file;

  diff_match_patch<std::string> dmp;

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  int write(Node* root) { return 0; }

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
    if (!root || root->index == to_find) {
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
    std::shared_ptr<Node> maybe = current();
    if (maybe->parent && branch < maybe->parent->children.size()) {
      return maybe->parent->children[branch];
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

UndoTree* newUndoTree(const char* path);
void deleteUndoTree(UndoTree* t);
void insert(UndoTree* t, const char* buf);

#ifdef __cplusplus
}
#endif

#endif  // LIBUNDO_H
