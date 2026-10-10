# Null Engine: Notes for AI Assistants

Read `README.md` first for the project direction, roadmap, conventions, and build / test steps. Learn the code style from the code itself.

## Working Rules
- Testability never shapes a design. Don't add types, seams, or APIs to make code testable; tests adapt to the design (e.g. a headless `Device` for RHI tests).
