# Agent Guidelines (Null Engine)

- **Root Documentation**: Always inspect and read `README.md` in the repository root for project architecture, roadmap milestones, coordinate conventions, and build instructions before planning or executing tasks.
- **Modern Industry Standards**: Always adhere to modern software engineering, C++20, and rendering industry standards and best practices.
- **Automated Verification**: Always perform automatic verification after code changes by executing the single chained build and test command specified in `README.md` for the active platform.
- **Efficient Operations**: Minimize token usage by scoping code search/grep strictly to `src/` and `shaders/` (never searching or parsing `build/`), using line-bounded file reads (`view_file`), and avoiding repetitive restatements of untouched code.
