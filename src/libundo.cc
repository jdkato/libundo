#include "libundo.h"

extern "C" {
/**
 * @brief      { function_description }
 *
 * @param[in]  buf   The buffer
 *
 * @return     { description_of_the_return_value }
 */
UndoTree* newUndoTree(const char* buf, const char* dir) {
  return reinterpret_cast<UndoTree*>(new UndoTree(buf, dir));
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
int insert(UndoTree* t, const char* buf) { return t->insert(buf); }
}
