#pragma once

#define WEASEL_CODE_NAME "Weasel"
#define WEASEL_REG_KEY L"Software\\Rime\\Weasel"
#define RIME_REG_KEY L"Software\\Rime"

// Optional private-use marker that Rime may embed in the commit string to
// request where the caret should land after the text is committed. Weasel
// strips it before the text reaches the document, then places the insertion
// point at that offset. This lets an input schema commit paired punctuation
// and leave the caret between the pair.
#define WEASEL_CARET_ANCHOR L'\ue000'

#define STRINGIZE(x) #x
#define VERSION_STR(x) STRINGIZE(x)
#define WEASEL_VERSION VERSION_STR(VERSION_MAJOR.VERSION_MINOR.VERSION_PATCH)
