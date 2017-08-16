#ifndef LIBUNDO_H
#define LIBUNDO_H

#include "diff_match_patch.h"

#include <fstream>
#include <iostream>
#include <stack>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

/**
 * { item_description }
 */
#define NODE_END ')'

/**
 * { item_description }
 */
#define NODE_START '('

/**
 * @brief      { struct_description }
 */
struct Node {
  int index;
  Node* parent;
  std::vector<Node*> children;
  std::pair<std::string, std::string> patches;
};

class UndoTree {
 public:
  /**
   * @brief      { function_description }
   *
   * @param[in]  hash  The hash
   * @param[in]  dir   The dir
   */
  UndoTree(const std::string& path) : root(NULL), total(0), branch(0), idx(0) {
    std::ifstream history(path);
    if (history.is_open()) {
      std::stack<Node*> stage;
      std::string patch;
      Node* parent = NULL;
      char ch;
      while (history >> std::noskipws >> ch) {
        if (ch == NODE_START) {
          patch = "";
          Node* temp = new Node;
          if (parent == NULL) {
            root = temp;
          } else {
            parent->children.push_back(temp);
          }
          stage.push(temp);
          parent = temp;
        } else if (ch == NODE_END) {
          stage.pop();
          if (!stage.empty()) {
            parent = stage.top();
          }
        } else {
          patch += ch;
        }
      }
    }
    history.close();
    undo_file = path;
  }

  /**
   * @brief      Destroys the object.
   */
  ~UndoTree(void) {
    write(root);
    clear(root);
  }

  /**
   * @brief      { function_description }
   *
   * @param[in]  buf   The buffer
   *
   * @return     { description_of_the_return_value }
   */
  void insert(const std::string& buf) {
    Node* to_add = new Node;
    to_add->index = ++total;

    if (!root) {
      root = to_add;
      root->patches = patch("", buf);
      root->parent = NULL;
    } else {
      Node* parent = find_parent();
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
  Node* current() { return search(root, idx); }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  std::vector<Node> nodes() { return collect(root); }

  /**
   * @brief      { function_description }
   */
  void switch_branch(void) {
    Node* pos = current();
    if (branch + 1 < pos->parent->children.size()) {
      branch = branch + 1;
    } else {
      branch = 0;
    }
  }

 private:
  Node* root;

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
  int write(Node* root) {
    std::ofstream outfile;
    outfile.open(undo_file);
    if (root != NULL) {
      outfile << "(" << root->index;
      for (auto child : root->children) {
        write(child);
      }
      outfile << ")";
    }
    outfile.close();
    return 0;
  }

  std::vector<Node> collect(Node* root) {
    std::vector<Node> collected;
    if (root != NULL) {
      collected.push_back(*root);
      for (auto child : root->children) {
        std::vector<Node> found = collect(child);
        collected.insert(collected.end(), found.begin(), found.end());
      }
    }
    return collected;
  }

  /**
   * @brief      { function_description }
   *
   * @param      root  The root
   *
   * @return     { description_of_the_return_value }
   */
  int clear(Node*& root) {
    if (!root) {
      return 0;
    };

    for (auto child : root->children) {
      clear(child);
    }

    delete root;
    root = NULL;

    return 1;
  }

  /**
   * @brief      Searches for the first match.
   *
   * @param      root     The root
   * @param[in]  to_find  To find
   *
   * @return     { description_of_the_return_value }
   */
  Node* search(Node* root, int to_find) {
    if (!root || root->index == to_find) {
      return root;
    }

    for (Node* child : root->children) {
      Node* found = search(child, to_find);
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
  Node* find_parent() {
    Node* maybe = current();
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
