# Migration Stages

> Status: seeded

## Summary

The migration should happen in stages rather than as a single rewrite. The system is large, the target stack introduces new infrastructure and tooling, and some key technical directions still need to be validated in practice. Each stage should prove a limited set of assumptions before the project moves deeper into more coupled and higher-risk parts of the system.

## Overall Strategy

- Why stage the migration:
  - The system is too large and too intertwined to migrate safely in one step.
  - The team is still building experience with PostgreSQL and the future stack.
  - Early stages should reduce uncertainty before the project touches more central user-facing data and workflows.
- What each stage should validate:
  - whether the chosen technical direction works in practice
  - whether the team can operate the new storage and migration workflow reliably
  - whether the boundaries between legacy and new components are workable
  - whether the migration path remains reversible when needed
- What should be avoided:
  - trying to migrate the most coupled or highest-risk subsystem first
  - treating exploratory implementation as if it already defines the final architecture
  - forcing all data domains and client paths to move at the same time

## Stage 1: Logging System Migration

- Goal:
  - Move legacy log text into PostgreSQL and build the helper path so new logs can also be written into the database.
- Scope:
  - Logging-related data only.
  - No attempt yet to migrate core user-facing content such as boards or articles.
- Why this stage comes first:
  - Logging is less tightly coupled to the core product behavior than articles and user-facing flows.
  - It is a lower-risk subsystem for validating the database direction.
- What it validates:
  - PostgreSQL integration in this project
  - initial schema design habits
  - migration tooling patterns
  - coexistence between legacy disk-based data and new database-backed data
  - backup and storage planning for a migrated data domain
- Exit criteria:
  - existing log data can be migrated into PostgreSQL
  - new logs can be recorded into PostgreSQL through a stable helper path
  - the team has enough confidence to apply similar patterns to other low-risk data domains
- Notes:
  - This stage is meant to prove feasibility, not to define the entire future data architecture.

## Stage 2: Structured Data And MySQL Retirement

- Goal:
  - Migrate relatively decoupled structured data into PostgreSQL and retire MySQL from the target runtime.
- Scope:
  - user data
  - board data
  - subscription-related data
  - article metadata
  - registration data currently stored in MySQL
- Candidate subsystem or data group:
  - These data domains are treated as a related group because they are structured, closely connected, and already have some migration history through earlier MySQL-based experiments.
  - Article metadata belongs in this stage because it is structured data and is used by subscription-related behavior.
- Why this stage is suitable:
  - It extends the database migration approach from logs into more important but still manageable structured data.
  - Some of this area has already been explored in previous experiments, which reduces uncertainty.
  - MySQL currently holds structured data that belongs in this stage, plus registration data.
- What it validates:
  - migration of more structured relational data
  - coexistence with remaining legacy storage
  - replacement of MySQL-backed pieces with PostgreSQL-backed equivalents
  - whether the new schema direction scales beyond the logging experiment
- Exit criteria:
  - user, board, subscription, article metadata, and registration data can be stored and read reliably through PostgreSQL-backed paths
  - all current MySQL-backed data has PostgreSQL-backed replacements
  - MySQL can be retired from the target runtime after validation
  - the team has a clearer model for relational schema evolution and migration tooling
- Notes:
  - This stage still avoids the full complexity of article bodies, attachment payloads, mail content, and content encoding.

## Stage 3: Article Content And Attachment Migration

- Goal:
  - Migrate article bodies into the new storage model and separate attachment payloads into object storage.
- Scope:
  - article bodies
  - attachment metadata
  - attachment payload storage
- Candidate subsystems or data domains:
  - article content and attachment storage
- What it validates:
  - handling of the most important content domain in the system
  - GBK and UTF-8 coexistence in real migrated content
  - attachment reference handling and storage separation
  - whether the planned PostgreSQL plus object storage model works for core content
- Exit criteria:
  - article bodies and attachments can be stored and accessed through the new storage architecture
  - the migration design for core content is stable enough to support later API-centered work
- Notes:
  - Article metadata is expected to have been handled in Stage 2.
  - This is expected to be one of the highest-risk stages because it touches the core content payload model.

## Stage 4: Mail Migration

- Goal:
  - Migrate mail data after the article content and attachment migration has validated the core content storage approach.
- Scope:
  - mail metadata
  - mail content
  - mail-related attachment handling if applicable
- Candidate subsystems or data domains:
  - direct mails and related personal message storage
- What it validates:
  - reuse of article migration patterns for a similar but distinct user data domain
  - whether the shared article-like data structure can be adapted cleanly for mail
  - whether personal/private content migration introduces additional operational or access-control concerns
- Exit criteria:
  - mail data can be stored and accessed through the new storage architecture
  - the project has a clearer model for migrating article-like domains beyond public board content
- Notes:
  - Mail is separated from article migration because it is a distinct user data domain even if it shares a similar data structure.

## Stage 5: API-Centered Feature Migration

- Goal:
  - Shift the API layer to consume the migrated data directly instead of relying on legacy GBK-oriented raw storage and conversion wrappers.
- Scope:
  - API-facing data access
  - UTF-8-oriented content delivery
  - selected features that can begin using the new storage path
- Candidate subsystems or feature groups:
  - API data access and early API-backed feature paths
- What it validates:
  - whether the new data model can support direct API consumption
  - whether UTF-8-first access paths simplify the current API behavior
  - whether feature migration can proceed incrementally rather than as a full cutover
- Exit criteria:
  - some API features run against the migrated storage path without depending on the old raw-content access pattern
  - the project has a clearer boundary between legacy access layers and new API-centered behavior
- Notes:
  - The exact API framework or language stack may still evolve during or after this stage.

## Stage 6: Client Transition And Legacy Retirement

- Goal:
  - Reverse-engineer the main flows from `bbs` and `nju09/www`, re-implement the preserved behavior through the API layer, and continue the transition away from legacy clients and daemons.
- Scope:
  - major user-facing flows
  - management-related flows selected for preservation
  - client transition work across web and future TUI paths
- Candidate subsystems or feature groups:
  - `bbs`
  - `nju09/www`
  - future API-backed web and TUI behavior
- What it validates:
  - whether the API layer can carry the preserved core workflows
  - whether the future clients can replace the important legacy access patterns
  - whether the project can retire major legacy runtime components without losing necessary functionality
- Exit criteria:
  - important preserved flows exist in the new architecture
  - the legacy runtime can begin real retirement rather than just coexistence
- Notes:
  - By this stage, parts of the API implementation stack may also have evolved beyond the earlier experimental shape.

## Stage Risks

- early success in low-risk migration areas may create false confidence about harder content and workflow migration
- transitional coexistence between legacy storage and new storage may last longer than expected
- schema or storage decisions made too early may need revision when core content migration begins
- retiring MySQL after Stage 2 depends on confirming that registration data and all other MySQL-backed domains have PostgreSQL-backed replacements

## Open Questions

- Should a future stage be reserved explicitly for background jobs and maintenance utilities, or should those be folded into the surrounding data and API migration stages?
- Which stage should own the first serious validation of the future TUI direction?
