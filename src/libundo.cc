#include "libundo.h"

extern "C" {
/**
* @brief      { function_description }
*
* @param[in]  buf   The buffer
*
* @return     { description_of_the_return_value }
*/
UndoTree* newUndoTree() { return reinterpret_cast<UndoTree*>(new UndoTree()); }

/**
 * @brief      { function_description }
 *
 * @param[in]  buf   The buffer
 *
 * @return     { description_of_the_return_value }
 */
UndoTree* loadUndoTree(const char* path) {
  UndoTree* t = new UndoTree();

  std::ifstream history(path, std::ios::in | std::ios::binary);
  if (history.is_open()) {
    cereal::BinaryInputArchive archive(history);
    archive(*t);
    history.close();
  }

  return reinterpret_cast<UndoTree*>(t);
}

/**
 * @brief      { function_description }
 *
 * @param      t     { parameter_description }
 */
void saveUndoTree(UndoTree* t, const char* path) {
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
 * @param[in]  buf   The buffer
 *
 * @return     { description_of_the_return_value }
 */
void insert(UndoTree* t, const char* buf) { t->insert(buf); }
}
