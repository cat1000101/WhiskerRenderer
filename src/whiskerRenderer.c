#include "whiskerRenderer.h"
#include "parser.h"
#include "utils.h"

W_Font *parseFont(MappedFile fontFile) {
    W_Font *font = SAFE_MALLOC(sizeof(W_Font));
    deserializedFont(fontFile, &font->parser);
    return font;
}
