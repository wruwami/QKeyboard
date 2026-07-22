# QKeyboard - common rules for AI agents

Applies to every AI agent working in this repository (Claude Code, GitHub
Copilot's coding agent, etc). Agent-specific session notes live elsewhere
(`.agents/` for Copilot); this file is the shared, cross-agent contract.

## Branching

### Branch naming

**Branch name format: `<ai-name>/issue-<number>-<short-work-description>`**

Examples:
- `antigravity/issue-12-ci-workflow-and-badges`
- `claude/issue-7-cmake-build-fix`
- `copilot/issue-3-ko-layout`

- Always include the **issue number** so it is immediately clear which
  issue the branch addresses and whether two agents are working on the
  same thing.
- Use **kebab-case** for the description part (no spaces, no slashes).
- Never commit directly to `main`.

### Before starting work

1. Run `git fetch origin` and pull the latest changes into `main` (`git checkout main && git pull`) to ensure your local baseline is fully up to date.
2. Check the remote Pull Requests status (e.g., using `gh pr list --state all --limit 10` or checking GitHub) to verify that the issue you are about to address is not already resolved, in review, or merged by another agent.
3. Check whether a branch for the same issue already exists on the remote:
   ```
   git branch -r | grep issue-<number>
   ```
4. If a branch already exists for that issue, **do not create a new one**.
   Check out the existing branch, review its commits, and continue from
   there instead of duplicating work.

### Before pushing

- Run `git fetch` again and confirm your branch has not moved on the remote.
- A rejected push means another agent has added commits; merge them in and
  re-verify before doing anything. **Never force-push over another agent's
  commits.**