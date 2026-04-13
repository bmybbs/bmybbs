# BMYBBS Guide

## Project Structure

This project contains multiple subprojects.

### C related

* `CMakeLists.txt` the root configuration for C related subprojects. `Makefile`s are saved for references.
* `/include` header files that shared across the project.
* `/libytht` contains helper functions from the project YTHTBBS, for example string operations.
* `/libbmy` also contains helper functions, but not part of the YTHTBBS.
* `/libythtbbs` implements many BBS related logic.
* `/src/bbsd` the telnet daemon.
* `/src/bbs` started by `bbsd`, implements the terminal user interface.
* `/smth_sshbbsd` the ssh daemon combining SSH 1.x and `/src/bbs`.
* `/nju09/www` a CGI implementation provide the web-based user interface for the BBS system.
* `/local_utl` contains utilities.
* `/api` an API layer which mirrors functionalities from `/src/bbs` and `/nju09/www`. Encoded in UTF-8, uses JSON.

### Other Subprojects

* `/randpics` written in Perl, provides a bacic UI to manage images displayed in sign-in page in `/nju09/www`.
* `/web` written in JavaScript, using Vue 3 as the framework for a modern looking, web-based user interface.

## Build and Testing

There is no need to run build commands or test commands. Human developers shall review changes, run build and tests manually.

## Important Notes

* Follow the indention and encoding if a file exists.
* When creating a new file, using UTF-8 and tab instead.
