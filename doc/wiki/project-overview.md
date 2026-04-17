# Project Overview

> Status: seeded

This system is a campus community platform for a single university. Its core purpose is to provide a bulletin board system where users can:

- register, log in, and query user information
- browse boards grouped by sections
- browse articles within a board, in threaded or non-threaded views
- read, post, and reply to articles
- send direct mails and short messages to other users

Administrators and board masters can also manage configurations and perform moderation-related tasks.

## Current Access Modes

The current system is exposed through multiple access layers:

- terminal access through Telnet and SSH
  - `bbsd` and `smth_sshbbsd` act as daemon servers
  - `bbs` provides the main terminal user interface and still carries many management-related workflows
- web access through HTTP and HTTPS
  - `nju09/www` is a CGI-based web interface with more limited functionality than the terminal interface
  - `api` is an experimental JSON-based access layer used by the modern `web` frontend

Some experimental or less important clients exist, but they are not central to the long-term plan.

## Why The Legacy System Is Hard To Maintain

Several different kinds of maintenance burden have accumulated in the legacy system.

- Architecture and storage
  - The filesystem is used as the primary data store.
  - Many records are stored as binary data mapped directly from C structs.
  - The system depends heavily on operating system facilities and low-level file management instead of a database handling those concerns.
- Coupling and duplication
  - Similar business logic is implemented multiple times across different access layers.
  - Many functions are tied too closely to specific interfaces instead of being organized as reusable domain logic.
- Data format constraints
  - Articles are encoded in GBK, while modern interfaces expect UTF-8.
  - Attachments are embedded inside article content, which makes storage and migration harder.
- Implementation debt
  - Some low-level code paths do not check system call results carefully enough, or check them incorrectly.
  - The codebase also contains historical work-arounds that made sense at the time but are now difficult to reason about or extend.

## Target Direction

The long-term goal is to retire the legacy access architecture and replace it with a simpler, more maintainable system centered on a single API layer.

The current planning direction is:

- preserve the core community workflows, especially user management, board view, and article view
- move primary data storage to PostgreSQL
- continue using Redis for caching
- likely move binary attachments to an object-storage-based design
- replace multiple legacy access layers with one API-oriented backend
- continue evolving the modern web frontend
- explore a new TUI client that provides some of the feel of `bbs` without preserving the old implementation model

Not every legacy feature will be migrated. For example, the SSH daemon based on SSH 1.x is expected to be retired, and game-related modules are likely to be dropped.

The exact target technologies are still planning decisions rather than final commitments. The legacy codebase will remain useful as a reference and may still receive critical bug fixes, but it is not intended to be the long-term foundation for new features.

## Scope Of This Wiki

This wiki is intended to support the migration from the legacy system to a new architecture.

Its goals are:

- capture stable understanding of the current system
- distinguish confirmed facts from inference and open questions
- record the intended target architecture and migration direction
- help map legacy subsystems and workflows into future equivalents

Detailed subsystem behavior, storage design, encoding issues, and migration planning should be split into their own pages as the wiki grows.
