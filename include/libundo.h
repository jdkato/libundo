#ifndef LIBUNDO_H
#define LIBUNDO_H

#include "diff_match_patch.h"

#include <fstream>
#include <iostream>
#include <stack>
#include <string>
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
  std::string f_patch;
  std::string b_patch;
  int index;
  Node* parent;
  std::vector<Node*> children;
};

class UndoTree {
 public:
  /**
   * @brief      { function_description }
   *
   * @param[in]  hash  The hash
   * @param[in]  dir   The dir
   */
  UndoTree(const std::string& path) : root(NULL), total(0) {
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
  int insert(const std::string& buf) {
    Node* to_add = new Node;
    to_add->index = ++total;
    if (!root) {
      root = to_add;
      root->f_patch = dmp.patch_toText(dmp.patch_make("", buf));
      root->b_patch = dmp.patch_toText(dmp.patch_make(buf, ""));
      root->parent = NULL;
    } else {
      /**
       *     1
       *   / | \
       *  2  3  4
       *    / \  \
       *   5   @  7
       */
      std::cout << "Adding " << buf << " with index " << to_add->index
                << std::endl;
      std::cout << "Looking for parent with index " << index << std::endl;
      Node* parent = search(root, index);
      std::cout << "Parent = " << parent->index << std::endl;
      parent->children.push_back(to_add);

      to_add->parent = parent;
      to_add->f_patch = dmp.patch_toText(dmp.patch_make(cur_buf, buf));
      to_add->f_patch = dmp.patch_toText(dmp.patch_make(buf, cur_buf));
    }
    index = to_add->index;
    cur_buf = buf;
    return 0;
  }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  std::string undo() {
    if (index - 1 >= 1) {
      index = index - 1;
      std::string patch = search(root, index)->b_patch;
      std::pair<std::string, std::vector<bool>> out =
          dmp.patch_apply(dmp.patch_fromText(patch), cur_buf);
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
    if (index + 1 <= total) {
      index = index + 1;
      std::string patch = search(root, index)->f_patch;
      std::pair<std::string, std::vector<bool>> out =
          dmp.patch_apply(dmp.patch_fromText(patch), cur_buf);
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

  std::vector<Node> nodes() { return collect(root); }

 private:
  Node* root;

  int total;
  int index;

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

  int clear(Node*& root) {
    if (!root) return 0;

    for (auto child : root->children) {
      clear(child);
    }

    delete root;
    root = NULL;

    return 1;
  }

  Node* search(Node* root, int to_find) {
    if (!root || root->index == to_find) {
      return root;
    } else {
      for (auto child : root->children) {
        return search(child, to_find);
      }
    }
  }
};

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UndoTree UndoTree;

UndoTree* newUndoTree(const char* path);
void deleteUndoTree(UndoTree* t);
int insert(UndoTree* t, const char* buf);

#ifdef __cplusplus
}
#endif

#endif  // LIBUNDO_H
