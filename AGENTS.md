# BMYBBS Guide

## Project Structure

This project contains multiple subprojects. In this file they are listed, not explained.

### C related

* `CMakeLists.txt`
* `include`
* `libytht`
* `libbmy`
* `libythtbbs`
* `src/bbsd`
* `src/bbs`
* `smth_sshbbsd`
* `nju09/www`
* `local_utl`
* `api`

### Other Subprojects

* `randpics`
* `web`

### Other Project Files

* `db`
* `doc`
  * `doc/wiki` is the active working wiki.
  * Other files or folders under `/doc` are legacy documents, do NOT read or rely on them unless the user explicitly asks.

## Build and Testing

There is no need to run build commands or test commands. Human developers shall review changes, run build and tests manually.

## Important Notes

* Read `doc/wiki/index.md` before substantial analysis.
* Treat `doc/wiki` as working memory; do not treat source code as the sole truth.
* Prefer updating existing wiki pages over repeating conclusions in chat.
* Use the wiki for accumulated understanding, but mark conflicts between wiki, code, and user statements explicitly.
* Record uncertainty explicitly.
* Follow the relevant skill when creating or revising wiki pages.
* Follow the indentation and encoding when editing an existing file.
* Use UTF-8 and tab indentation when creating a new file.
