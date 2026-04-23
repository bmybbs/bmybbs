# Target Architecture

> Status: seeded

## Summary

The future system should replace the legacy access architecture with a modular system centered on a single API layer. The main goals are to preserve the core community workflows, reduce dependence on low-level operating system mechanisms, and make future development easier than in the current monolithic legacy codebase. The target architecture should support both a modern web client and a future TUI client, while keeping those clients separate from core business logic. Not every detail is decided yet, especially around API style and UI design, so this page records direction and boundaries rather than a finished technical design. The future API and client model will be designed partly by reverse-engineering legacy behavior and then reshaping it for a more maintainable system.

## Goals

- Build the future system on mature components and frameworks so the project can focus on BMYBBS itself instead of re-implementing operating-system-level concerns.
- Separate storage, business logic, access control, and client presentation into clearer layers.
- Preserve the core community workflows, especially user management, board view, and article view.
- Make the new architecture flexible enough to absorb later modules and revised decisions without forcing large rewrites.
- Reduce duplication across clients by moving shared logic into the backend.

## Proposed System Shape

The future system is expected to consist of several major parts with clearer boundaries than the legacy system.

- API layer:
  - The API layer should become the main access entry point for user-facing and management-related functionality.
  - It should cover the important capabilities now spread across `bbs`, `nju09/www`, and `api`.
  - It does not need to mirror every legacy feature in the first release.
- Data storage:
  - Primary structured data should move into PostgreSQL.
  - The storage design should support a transition period in which legacy filesystem-backed data, MySQL-backed data, and new PostgreSQL-backed data may coexist.
- Cache layer:
  - Cache and shared state should move away from the current shared-memory-oriented model.
  - Redis remains a likely choice because it is already used and fits the intended direction.
- Attachment storage:
  - Binary attachments should be managed separately from the main relational data store.
  - The attachment model should support safe reference tracking, deletion, and integrity checks.
  - The attachment model should not depend on the legacy article-embedding approach as a permanent design.
- Web client:
  - The current `web` frontend is the likely base for the future browser-facing client.
  - Exact UI design is still open and should not block backend architectural decisions.
- TUI client:
  - A future TUI client is intended to provide part of the `bbs` experience in a more portable and modern form.
  - Its exact interaction design remains exploratory.
- Background jobs and maintenance components:
  - Some responsibilities currently hidden in legacy daemons or scripts may need to become explicit background jobs, maintenance tools, or migration utilities.
  - Likely examples include hot or top thread generation, scheduled user undeny operations, and statistical or maintenance utilities currently driven by cron-style workflows.

## Responsibilities By Layer

### API Layer

What it should do:

- expose the core system capabilities through a stable programmatic interface
- centralize authentication, authorization, and permission checks
- encapsulate business logic that is currently duplicated across multiple access paths
- return client-facing data in a format suitable for modern consumers, likely JSON-first
- support incremental delivery so different feature sets can be released in stages

What it should not do:

- depend on client-specific presentation logic
- duplicate low-level storage logic across multiple service entry points
- expose privileged operations without explicit authorization checks
- become tightly coupled to one client implementation or one UI style

### Clients

What clients should do:

- focus on presentation, interaction flow, and client-side user experience
- call the API layer instead of re-implementing business rules
- adapt the same backend capabilities to different usage contexts such as browser and terminal

What clients should not do:

- carry their own copies of permission logic or domain rules
- depend directly on legacy storage layout or low-level system interfaces
- become the main place where cross-cutting backend behavior is implemented

### Data And Storage Layer

What belongs here:

- structured persistent data such as users, boards, articles, configuration, and operational records
- compatibility considerations needed during migration from legacy formats to new representations
- query patterns and indices needed for commonly used access paths
- attachment metadata and references, even if binary payloads are stored outside the main database

This layer may need transitional designs for encoding, schema evolution, and data migration rather than a single clean cut from old to new.

During transition, both GBK and UTF-8 representations may need to coexist. GBK should remain the raw compatibility form for the legacy system, while UTF-8 becomes the future-facing representation for the new API and clients. Write paths may need to synchronize both forms until the legacy runtime is fully retired.

## Expected Retirements

The long-term architecture is expected to retire several legacy runtime components.

- the legacy Telnet and SSH access daemons as primary service entry points
- connection models that require a user-facing server process to die with the session and lose in-progress state on disconnect
- the CGI-based web access layer
- legacy daemon patterns that exist mainly to support old runtime assumptions
- the old standalone chat-related service model where it no longer fits the unified architecture
- legacy game-related modules that are not part of the future product direction

The retirement of the Telnet and SSH access model is intentional. Telnet is not acceptable as a long-term remote access method because it is unencrypted, and the existing SSH implementation is tightly tied to SSH 1.x. More importantly, the old long-lived connection model couples the user session too closely to one server-side process. The future system should allow both short-lived request/response interactions and longer-lived channels such as WebSocket where appropriate, without making client stability depend on one legacy terminal session process.

The legacy codebase may still remain as a reference and may still receive critical fixes during transition, but it is not the intended long-term runtime base.

## Planned Technology Direction

Things that are likely:

- PostgreSQL as the main structured data store
- gradual retirement of legacy MySQL-backed pieces as PostgreSQL-backed replacements become available
- Redis as the main caching layer
- an object-storage-oriented design for binary attachments
- Rust as a strong candidate for at least part of the new implementation stack, especially where safer low-level systems work or reusable core libraries are valuable
- a backend architecture that does not depend directly on `libythtbbs` and `libytht` as long-term foundations

Things that are still open or exploratory:

- the exact implementation stack for the future API layer
- whether Rust becomes the main implementation language for the new backend or is introduced first in selected lower layers
- the exact client architecture for the future web frontend
- the final design and scope of the future TUI client
- the concrete object storage product or deployment model

## Migration Principles

The migration should be guided by a small set of architectural principles rather than by one-time rewrites.

- preserve important data and core workflows before chasing feature completeness
- favor modular boundaries so parts of the new system can evolve independently
- allow staged migration instead of requiring a single cutover
- keep decisions reversible where the target design is still uncertain
- record important tradeoffs and decision history so later iterations remain understandable
- reduce dependence on legacy OS-level mechanisms whenever a mature infrastructure component can take over that responsibility
- derive new API behavior from legacy behavior through reverse-engineering and redesign, rather than copying legacy interfaces blindly

## Open Questions

- After the legacy runtime is retired, how long should GBK data continue to be retained for compatibility or historical backup purposes?
- Which management and administrative functions must be present in the earliest API releases?
- Should the future backend be designed as one deployable service first, or as several internal modules with clearer boundaries inside one service?
- What attachment lifecycle model best fits migration, deletion safety, and future extensibility?
- Which parts of the current operational workflow still require dedicated background services in the new architecture?
