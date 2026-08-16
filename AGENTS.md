# Agent Guidelines (Null Engine)

- **Root Documentation**: Always inspect and read `README.md` in the repository root for project architecture, roadmap milestones, coordinate conventions, and build instructions before planning or executing tasks.
- **Modern Industry Standards**: Always adhere to modern software engineering, C++20, and rendering industry standards and best practices.
- **Library Isolation**: Confine third-party math dependencies (such as GLM) strictly within `src/math/`—never leak external library headers or types into components, rendering, or tests.
- **Automated Verification**: Always perform automatic verification by building the project and running the unit tests after making code changes.
