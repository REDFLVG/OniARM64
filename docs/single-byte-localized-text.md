# Single-byte retail text decoding

Russian retail data stores menu strings as one-byte glyph indices. In the tested data, `WMDDdialog_mainmenu` contains Windows-1251 bytes for Новая Игра, and `TSFFTahoma` references `TSFLRoman`. Its fonts have glyphs in page zero, including the upper byte range; they do not have the pages that would contain paired Cyrillic bytes.

The old decoder treated every lead byte from 0x80 through 0xfe as the start of a two-byte character. For example, the bytes 0xcd 0xee became glyph code 0xcdee instead of two glyphs 0x00cd and 0x00ee. This also incorrectly paired the leading 0x81 menu decoration with the first letter. The selected glyph pages did not exist, so the buttons were blank. Switching renderers could not change that decoding.

After level zero is loaded, `TSrLevel_Load` checks the default font family's language resource. `Roman` uses single-byte decoding throughout the existing text/string routines. Other and missing language resources keep the legacy two-byte decoder, including its 0xff caret exception. A dangling lead byte no longer consumes the NUL terminator. No Unicode conversion, font replacement, asset repacking, or sound modification is required.

## Validation

Run `tests/test_text_encoding.sh` on macOS with SDL2 headers installed. It compiles the real text-system translation unit, stubs only resource lookup/logging/string-copy support, and removes unused rendering routines at link time. AddressSanitizer and UndefinedBehaviorSanitizer cover 279 checks: all non-NUL Roman bytes, Russian labels including 0xff, ASCII, insertion/deletion, glyph-based measurement, single-line layout, multiline wrapping, Asian paired bytes, the caret exception, dangling lead bytes, unknown/missing language resources, and mode reset.

The Russian retail data was tested locally with a native arm64 build: the four main-menu labels and Russian load-game controls render, and the training level loads. The original extracted and installed GameDataFolder contents matched for all 92 files before testing. Runtime tracing confirmed single-byte Cyrillic glyph codes and successful ADPCM decoding. Manual listening and the complete campaign are separate checks; this is not a claim of end-to-end campaign validation.

The existing shared texture cache was left unchanged because the tested failure occurs before glyph caching. Asian, English, and other European retail data sets were not available for visual checks; their decoder behavior is covered by the regression test.

## References

- [OniGalore: text encoding](https://wiki.oni2.net/OBD:Text_encoding)
- [Daodan language patches](https://wiki.oni2.net/Daodan_DLL#All_configuration_settings)
- [German demo text issue #135](https://github.com/andiyar/OniARM64/issues/135) (similar reported symptoms, not separately reproduced)
