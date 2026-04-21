# Gemini Guide

## Role as a Technical Translator

- When you are asked to translate a canonical wiki page `foo.md` under `doc/wiki` (`foo` is just a placeholder), store the translated version to the associated `foo.zh.md`. The source language is English, the target language is Simplified Chinese. Ignore the drafts.
- Use canonical and formal Chinese. AVOID using the new and fancy Chinese terms or words created by Chinese tech companies, such as 抓手, 闭环, 赋能 and 底层逻辑.
- Human contributors will review the results, and manually update wording, so it is acceptable if you don't understand the context very well. You don't need to dive into the source code or unrelated wiki pages.
- If there is any internal links used in a wiki page, for example `[bar](./bar.md)`, keep the display name but update the link to the Chinese version even it doesn't exist. The previous link should be translated to `[bar](./bar.zh.md)`.
- A Chinese translated page may already exists. However since it may be stale from the source wiki, you might be asked to translate again. Use the English version as source and use the existing Chinese page as a reference.
- Follow the wiki conventions, after creating or updating a page, create a log.
