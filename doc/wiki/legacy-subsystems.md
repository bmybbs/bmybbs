# Legacy Subsystems

> Status: seeded

## Summary

The legacy system is split across several user-facing access layers, service daemons, shared libraries, and supporting utilities. Some of these subsystems provide direct user access, while others exist mainly to support the legacy runtime model or to share common logic. This page is intended to describe the major subsystem boundaries at a high level, not to document detailed code paths. It is a map of the current system shape that can later be refined through code reading and migration planning.

## Subsystem List

The main legacy subsystems covered by this page are:

- `bbsd`
- `bbs`
- `smth_sshbbsd`
- `nju09/www`
- `api`
- `web`
- supporting libraries such as `libythtbbs`, `libytht`, and `libbmy`

## Current Roles

### `bbsd`

- Main role:
  - Telnet daemon that accepts incoming terminal connections and starts `bbs`.
- Access path:
  - Telnet requests sent to the configured BBS ports.
- Notes:
  - It listens on multiple ports.
  - One path supports BIG5 mode, but the main execution flow is similar.

### `bbs`

- Main role:
  - Main legacy service binary that implements most of the terminal-facing BBS behavior.
- Access path:
  - Spawned by `bbsd`, and also entered indirectly through `smth_sshbbsd`.
- Notes:
  - It is the central implementation for many legacy user and management workflows.

### `smth_sshbbsd`

- Main role:
  - SSH daemon and SSH-based entry point into the `bbs` execution flow.
- Access path:
  - SSH requests sent to port `22`.
- Notes:
  - One process listens for incoming SSH connections.
  - A child process serves each connection and then transitions into the `bbs` side of the system.
  - It combines SSH 1.x code with `bbs` source code, so the flow is related to `bbs` but not identical.

### `nju09/www`

- Main role:
  - Legacy CGI-based web interface.
- Access path:
  - HTTP requests routed by an external web server to the CGI entry point.
- Notes:
  - The web server handles routing to the CGI binary.
  - The CGI layer then parses the request and dispatches to the matching function.
  - This interface provides browser access but is more limited than the terminal interface.

### `api`

- Main role:
  - Experimental HTTP-based API layer.
- Access path:
  - Listens on its own port and is expected to be reverse proxied by an HTTP server.
- Notes:
  - It provides an alternative access path to BBS functionality.
  - It is currently used mainly together with `web`.

### `web`

- Main role:
  - Modern web frontend intended to work with `api`.
- Access path:
  - Served by the HTTP server as a browser-facing frontend.
- Notes:
  - It is not a standalone backend.
  - It represents the direction of the future browser-facing client more than the legacy CGI layer does.

### Supporting Libraries Or Shared Layers

- `libythtbbs`:
  - Provides core data structures and shared functions related to BBS flows.
  - Used by `bbs`, `nju09/www`, and utilities under `local_utl`.
- `libytht`:
  - Provides lower-level helper functions not tightly bound to one BBS flow.
  - Examples include string operations, file operations, random utilities, crypto helpers, time functions, and other common helpers.
- `libbmy`:
  - Provides project-specific helpers that do not come from the upstream YTHT project.
  - Examples include cookie management, MySQL wrappers, SMTP support, and 2FA-related functions.
- `include`:
  - Collects shared header files used by these libraries and related code.

## Known Relationships

At a high level, the current subsystem relationships are:

- `bbsd` starts `bbs` for Telnet-based access.
- `smth_sshbbsd` embeds or transitions into `bbs` logic for SSH-based access.
- `nju09/www` provides a separate CGI-based web path into similar or overlapping system behavior.
- `api` provides a newer HTTP-based access path and currently works mainly with `web`.
- the legacy system is not purely filesystem-backed; some newer or auxiliary pieces also depend on MySQL-backed storage paths.
- `web` depends on `api` rather than on the legacy CGI layer.
- shared libraries such as `libythtbbs`, `libytht`, and `libbmy` support multiple executables and utilities across the repository.

## Likely Migration Fate

Based on the current migration direction, these subsystems do not all have the same future.

- `bbsd`, `bbs`, `smth_sshbbsd`, and `nju09/www` are expected to be retired as long-term runtime components.
- `api` is not the final target architecture, but it is closer to the future API-oriented direction than the older access layers.
- `web` is likely to evolve rather than be retired immediately, although its exact future shape is still open.
- `libythtbbs`, `libytht`, and `libbmy` should be treated carefully: some parts may remain useful during migration, some parts may be replaced, and some parts may become reference-only.
- the legacy codebase as a whole remains an important reference during migration, even where it is no longer the long-term runtime foundation.

## Open Questions

- Which parts of `bbs` represent essential business behavior, and which parts are mainly terminal-specific interaction logic?
- Which parts of `libythtbbs`, `libytht`, and `libbmy` should be preserved, wrapped, replaced, or retired?
- Are there important legacy utilities or daemons outside these top-level subsystems that should also appear on this page in a later revision?
