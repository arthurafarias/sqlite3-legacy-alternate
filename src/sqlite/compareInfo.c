#define _GNU_SOURCE 1
#include "sqlite/compareInfo.h"
#include "sqlite/u8.h"
const struct compareInfo globInfo = {'*', '?', '[', 0};

const struct compareInfo likeInfoNorm = {'%', '_', 0, 1};

const struct compareInfo likeInfoAlt = {'%', '_', 0, 0};
