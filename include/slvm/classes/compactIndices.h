#ifndef SLVM_CLASSES_COMPACT_INDICES_H
#define SLVM_CLASSES_COMPACT_INDICES_H

enum SLVM_KernelClassIndex
{
#define COMPACT_CLASS_ALIAS_INDEX(className, index)
#define COMPACT_CLASS_INDEX(className, index) \
    SLVM_KCI_ ## className = index, \
    SLVM_KMCI_ ## className = 256 + index,

#include "compactIndices.inc"

#undef COMPACT_CLASS_INDEX
#undef COMPACT_CLASS_ALIAS_INDEX
};

#endif /* SLVM_CLASSES_COMPACT_INDICES_H */
