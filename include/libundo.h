#ifndef LIBUNDO_H
#define LIBUNDO_H

#include "diff_match_patch.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

/**
 * { item_description }
 */
#define NODE_END ')'

/**
 * @brief      { struct_description }
 */
struct Node {
  int index;
  std::string patch;
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
  UndoTree(const char* path) {
    FILE* fp = std::fopen(path, "r");
    if (fp != NULL) {
      // We found an undo file.
      read(root, fp);
    }
    fclose(fp);
  }

  /**
   * @brief      Destroys the object.
   */
  ~UndoTree(void) { remove_all(root); }

  /**
   * @brief      { function_description }
   *
   * @param[in]  buf   The buffer
   *
   * @return     { description_of_the_return_value }
   */
  int insert(const std::string& buf) { return append(head, buf); }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  std::string undo() { return ""; }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  std::string redo() { return ""; }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  int size() { return index; }

 private:
  Node* head;
  Node* root;

  int index;
  std::string cur_buf;

  diff_match_patch<std::string> dmp;

  /**
   * @brief      { function_description }
   *
   * @param      root  The root
   * @param[in]  buf   The buffer
   */
  int append(Node*& root, const std::string& buf) {
    if (!head) {
      head = new Node;
      head->patch = dmp.patch_toText(dmp.patch_make("", buf));
      head->index = 0;
      root = head;
    } else {
      Node* temp = new Node;
      head->children.push_back(temp);

      temp->index = head->index++;
      temp->parent = head;
      head = temp;

      head->patch = dmp.patch_toText(dmp.patch_make(cur_buf, buf));
      cur_buf = buf;

      delete temp;
    }

    return 0;
  }

  /**
   * @brief      Removes all.
   *
   * @param      root  The root
   *
   * @return     { description_of_the_return_value }
   */
  int remove_all(Node*& root) {
    if (!root) {
      return 0;
    };

    for (auto& branch : root->children) {
      remove_all(branch);
    }

    delete root;
    root = NULL;

    return 1;
  }

  /**
   * @brief      { function_description }
   *
   * @param      root  The root
   * @param      fp    { parameter_description }
   */
  int read(Node*& root, FILE* fp) {
    if (!fscanf(fp, "%c ", &val) || val == NODE_END) {
      return 1;
    }

    // Else create node with this item and recur for children
    root = newNode(val);
    for (int i = 0; i < N; i++)
      if (read(root->child[i], fp)) break;

    // Finally return 0 for successful finish
    return 0;
  }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  int write(FILE* fp) { return 0; }
};

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UndoTree UndoTree;

UndoTree* newUndoTree(const char* buf, const char* dir);
void deleteUndoTree(UndoTree* t);
int insert(UndoTree* t, const char* buf);

#ifdef __cplusplus
}
#endif

#endif  // LIBUNDO_H
