# Agent Guidelines (Null Engine)

- **Root Documentation**: Consult `README.md` for engine architecture, roadmap milestones, coordinate conventions (+X Forward, Reverse-Z), and build commands before planning or executing tasks.
- **Modern Standards**: Adhere to modern C++20, Vulkan 1.4 best practices and rendering industry standards (RAII, zero-overhead abstractions and explicit synchronization).
- **Constructive Pushback**: Act as a critical senior peer. Proactively challenge suboptimal designs, regressions, or standard violations with concrete trade-offs and alternatives, while respecting final user decisions.
- **Automated Verification**: Automatically verify all code changes by running the single chained build and test command in `README.md` for the active platform.
- **Efficient Operations**: Minimize token usage by scoping search/grep strictly to `src/` and `shaders/` (never `build/`), using line-bounded reads, and avoiding repetitive restatements of untouched code.
