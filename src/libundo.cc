#include "libundo.h"

extern "C" {
/**
 * @brief      { function_description }
 *
 * @param[in]  buf   The buffer
 *
 * @return     { description_of_the_return_value }
 */
UndoTree* newUndoTree(const char* path) {
  return reinterpret_cast<UndoTree*>(new UndoTree(path));
}

/**
 * @brief      { function_description }
 *
 * @param      t     { parameter_description }
 */
void deleteUndoTree(UndoTree* t) { delete t; }

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
