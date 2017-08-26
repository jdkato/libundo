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

#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace libundo {

struct Delta {
  std::string buffer;
  std::string patch;
  int position;
};

/**
 * @brief      { struct_description }
 */
class Node {
 public:
  int id;
  int position;

  std::shared_ptr<Node> parent;

  std::vector<std::shared_ptr<Node>> children;
  std::map<int, std::string> patches;

  std::string timestamp;

 private:
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
    ar(id, parent, children, patches, timestamp);
  }
};

class UndoTree {
 public:
  /**
   * @brief      { function_description }
   */
  UndoTree() : root(NULL), total(0), n_idx(0), b_idx(0) {}
  ~UndoTree() {}

  /**
   * @brief      Inserts the string `buf` into the tree.
   *
   * @param[in]  buf   The buffer contents.
   *
   */
  void insert(const std::string& buf, int pos = 0) {
    std::shared_ptr<Node> to_add = std::make_shared<Node>();

    to_add->id = ++total;
    to_add->timestamp = get_time();
    to_add->position = pos;
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
   * @brief      Undo the last insertion into the tree by moving back to its
   *             parent.
   *
   * @return     The contents of the previous Node -- i.e., the current buffer.
   */
  Delta undo() {
    std::shared_ptr<Node> parent = current_node()->parent;
    Delta dt;
    if (parent) {
      dt.patch = apply_patch(parent->id);
      dt.position = parent->position;
    } else {
      dt.patch = "";
      dt.position = current_node()->position;
    }
    dt.buffer = cur_buf;
    return dt;
  }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  Delta redo() {
    std::shared_ptr<Node> n = current_node();
    Delta dt;
    if (n->children.size() > 0) {
      dt.patch = apply_patch(n->children[b_idx]->id);
      dt.position = n->children[b_idx]->position;
    } else {
      dt.patch = "";
      dt.position = n->position;
    }
    dt.buffer = cur_buf;
    return dt;
  }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  size_t size() { return total; }

  /**
   * @brief      { function_description }
   *
   * @return     { description_of_the_return_value }
   */
  size_t branch() { return b_idx; }

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
  std::vector<Node> nodes() {
    std::vector<Node> collected;
    for (auto const& node : index) {
      collected.push_back(*node.second.get());
    }
    return collected;
  }

  /**
   * @brief      { function_description }
   */
  void switch_branch(int direction) {
    size_t n = current_node()->children.size();
    if (n <= 1) {
      // We have no where to move - e.g., [b_idx].
      return;
    } else if (direction != 0 && b_idx + 1 < n) {
      // We're moving to the right - e.g., [b_idx, b, c] => [a, b_idx, c].
      // This can only happen if we're not to the end of the list yet.
      b_idx = b_idx + 1;
    } else if (direction == 0 && b_idx > 0) {
       // We're moving to the left - e.g., [a, b_idx, c] => [b_idx, a, c].
       // This can only happen if we're not at the start of the list.
      b_idx = b_idx - 1;
    } else {
      // We're at the end of head's children (either extreme); wrap around
      // to the other end.
      b_idx = (b_idx == n - 1) ? 0 : n - 1;
    }
  }

 private:
  std::shared_ptr<Node> root;
  std::map<int, std::shared_ptr<Node>> index;

  size_t total;
  size_t n_idx;
  size_t b_idx;

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
    cur_buf = out.first;
    return patch;
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

  std::string get_time() {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    return oss.str();
  }
};

/**
 * @brief      { function_description }
 *
 * @param      t     { parameter_description }
 * @param[in]  path  The path
 */
inline void save(UndoTree* t, const std::string& path) {
  std::ofstream history(path, std::ios::out | std::ios::binary);
  if (history.is_open()) {
    cereal::BinaryOutputArchive archive(history);
    archive(*t);
    history.close();
  }
}

/**
 * @brief      { function_description }
 *
 * @param[in]  path  The path
 * @param[in]  buf   The buffer
 *
 * @return     { description_of_the_return_value }
 */
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
      return new UndoTree();
    }
  }

  return t;
}
}  // namespace libundo

#endif  // LIBUNDO_H
