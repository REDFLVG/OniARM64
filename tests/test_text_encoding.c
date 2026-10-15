// Exercise the real text system, including public string and layout routines.
// Resource lookup is stubbed; unused rendering code is removed by the linker.
#include "../BungieFrameWork/BFW_Source/BFW_TextSystem/BFW_TextSystem.c"
#include <stdio.h>

static TStFontLanguage language;
static TStFontFamily family;
static const char *language_name;
static UUtBool have_family = UUcTrue;
static int checks;

void *TMrInstance_GetFromName(TMtTemplateTag tag, const char *name)
{
	return have_family && tag == TScTemplate_FontFamily &&
		strcmp(name, TScFontFamily_Default) == 0 ? &family : NULL;
}

char *TMrInstance_GetInstanceName(const void *instance)
{
	return instance == &language ? (char *)language_name : NULL;
}

void UUcArglist_Call UUrStartupMessage(const char *format, ...) { (void)format; }

void UUrString_Copy_Length(char *dst, const char *src, UUtUns32 capacity, UUtUns32 count)
{
	size_t n = strlen(src);
	if (n > count) n = count;
	if (n > capacity) n = capacity;
	memcpy(dst, src, n);
	if (n < capacity) dst[n] = 0;
}

void UUrString_Copy(char *dst, const char *src, UUtUns32 capacity)
{
	if (capacity) UUrString_Copy_Length(dst, src, capacity, capacity - 1);
}

#define CHECK(condition) do { \
	checks++; \
	if (!(condition)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); return 1; } \
} while (0)

int main(void)
{
	char text[256];
	char edited[32] = "\xcd\xee\xe2\xe0\xff"; // Новая, Windows-1251
	TStGlyphArray glyphs = {0};
	TStFont font = {0};
	TStTextContext context = {0};
	UUtRect rect;
	UUtRect bounds = {0, 0, 48, 100};
	IMtPoint2D destination = {0, 10};
	TStStringFormat formatted;
	const char *menu = "\xcd\xee\xe2\xe0\xff \xc8\xe3\xf0\xe0";

	family.font_language = &language;
	language_name = "Roman";
	TSrLevel_Load();
	CHECK(TSrString_GetLength("New Game") == 8);
	CHECK(TSrString_GetLength("\xcd\xee\xe2\xe0\xff \xc8\xe3\xf0\xe0") == 10);
	// Every non-NUL byte, including 0x80/0x81/0xff, is one Roman glyph.
	for (int i = 1; i < 256; i++) text[i - 1] = (char)i;
	text[255] = 0;
	CHECK(TSrString_GetLength(text) == 255);
	for (int i = 0; i < 255; i++)
		CHECK(TSrString_GetCharacterAtIndex(text, i) == i + 1);
	CHECK(TSrString_GetCharacterAtIndex(edited, 4) == 0xff);
	CHECK(TSrString_DeleteChar(edited, 1, sizeof edited));
	CHECK(strcmp(edited, "\xcd\xe2\xe0\xff") == 0);
	CHECK(TSrString_InsertChar(edited, 1, 0xee, sizeof edited));
	CHECK(strcmp(edited, "\xcd\xee\xe2\xe0\xff") == 0);

	font.glyph_arrays[0] = &glyphs;
	font.ascending_height = 10;
	font.descending_height = 2;
	for (int i = 1; i < 256; i++) glyphs.glyphs[i].kerning = 8;
	context.font = &font;
	CHECK(TSrContext_GetStringRect(&context, edited, &rect) == UUcError_None);
	CHECK(rect.right == 40 && rect.bottom == 12);
	context.font_family = &family;
	context.format = TSc_SingleLine;
	CHECK(TSrContext_FormatString(&context, menu, &bounds, &destination, &formatted) == UUcError_None);
	CHECK(formatted.num_characters[0] == 10);
	CHECK(strcmp(formatted.segments[0], menu) == 0);

	// A narrow multiline subtitle must wrap without swallowing Cyrillic bytes.
	strcpy(language.breaking_2, " ");
	context.format = TSc_HLeft;
	CHECK(TSrContext_FormatString(&context, menu, &bounds, &destination, &formatted) == UUcError_None);
	CHECK(formatted.num_lines > 1);
	text[0] = 0;
	for (int i = 0; i < formatted.num_segments; i++) strcat(text, formatted.segments[i]);
	CHECK(strcmp(text, menu) == 0);

	language_name = "Japanese";
	TSrLevel_Load();
	CHECK(TSrString_GetLength("A\x82\xa0" "B") == 3);
	CHECK(TSrString_GetCharacterAtIndex("A\x82\xa0" "B", 1) == 0x82a0);
	CHECK(TSrString_GetLength("\xff" "A") == 2); // legacy caret exception
	CHECK(TSrString_GetLength("\x82") == 1); // dangling lead byte
	CHECK(TSrString_GetLength("") == 0); // one-byte NUL buffer
	language_name = "Unknown";
	TSrLevel_Load();
	CHECK(TSrString_GetLength("\x82\xa0") == 1);
	have_family = UUcFalse;
	TSrLevel_Load();
	CHECK(TSrString_GetLength("\x82\xa0") == 1);
	have_family = UUcTrue;
	language_name = "Roman";
	TSrLevel_Load();
	CHECK(TSrString_GetLength("\x82\xa0") == 2); // mode resets on reload
	printf("test_text_encoding: %d checks passed\n", checks);
	return 0;
}
