# .stheme parser and package validator

The parser, package/container reader, asset validator, bounded collection and streaming catalogue helpers in this directory are extracted from:

- Upstream: https://github.com/giovi321/smalltv-mod
- Release: `v3.13.0`
- Tag commit: `5396e3f80ce868e0fca2f7bbc032df16e66daa7a`

Included upstream components: `ThemeEngine` (manifest model/parser and engine source), `ThemePackage` (STH1/STI1/STI2/STF1 validation and asset access), `ThemeCatalog`, `BlockList`, `BudgetAllocator`, `Languages`, and `JsonStream`.

The web-only port exposes package upload, validation, listing and deletion. It does **not** activate or render themes; the display remains dark. The theme data-source fetcher, display adapter, clock/NTP integration, renderer and theme-selection mode are not included.

The extracted upstream files are distributed under the WTFPL 2.0 license; see `LICENSE-WTFPL-2.0.txt`.
