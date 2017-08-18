#include "libundo.h"

#include <functional>

extern "C" {
/**
 * @brief      { function_description }
 *
 * @param[in]  buf   The buffer
 *
 * @return     { description_of_the_return_value }
 */
UndoTree* new_tree() { return reinterpret_cast<UndoTree*>(new UndoTree()); }

/**
 * @brief      { function_description }
 *
 * @param[in]  buf   The buffer
 *
 * @return     { description_of_the_return_value }
 */
UndoTree* load_tree(const char* path, const char* buf) {
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
      free_tree(t);
      return new_tree();
    }
  }

  return reinterpret_cast<UndoTree*>(t);
}

/**
 * @brief      { function_description }
 *
 * @param      t     { parameter_description }
 */
void save_tree(UndoTree* t, const char* path) {
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
 * @param      t     { parameter_description }
 */
void free_tree(UndoTree* t) { delete t; }

/**
 * @brief      { function_description }
 *
 * @param      t     { parameter_description }
 * @param[in]  buf   The buffer
 *
 * @return     { description_of_the_return_value }
 */
void insert(UndoTree* t, const char* buf) { t->insert(buf); }

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
const char* undo(UndoTree* t) { return t->undo().c_str(); }

/**
 * @brief      { function_description }
 *
 * @param      t     { parameter_description }
 *
 * @return     { description_of_the_return_value }
 */
const char* redo(UndoTree* t) { return t->redo().c_str(); }

/**
 * @brief      { function_description }
 *
 * @param      t     { parameter_description }
 */
void switch_branch(UndoTree* t) { t->switch_branch(); }

/**
 * @brief      { function_description }
 *
 * @param      t     { parameter_description }
 *
 * @return     { description_of_the_return_value }
 */
const char* buffer(UndoTree* t) { return t->buffer().c_str(); }

/**
 * @brief      { function_description }
 *
 * @param      t     { parameter_description }
 *
 * @return     { description_of_the_return_value }
 */
int size(UndoTree* t) { return t->size(); }

/**
 * @brief      { function_description }
 *
 * @param      t     { parameter_description }
 *
 * @return     { description_of_the_return_value }
 */
int branch(UndoTree* t) { return t->branch(); }
}
