#!/usr/bin/env python3
"""Build a CHANGELOG.md suggestion prompt and post the AI result as a PR comment.

The model call itself is delegated to the `actions/ai-inference` GitHub Action
(GitHub Copilot CLI), so this script is split into two subcommands:

  build-prompt  Gather the PR diff + CHANGELOG.md context and write the system
                and user prompt files consumed by actions/ai-inference.
  post-comment  Read the model response produced by actions/ai-inference and
                upsert a single review comment on the PR.

Only GITHUB_TOKEN is required — no extra secrets or third-party dependencies.
"""

from __future__ import annotations

import json
import os
import sys
import urllib.error
import urllib.request
from datetime import date

# ── constants ────────────────────────────────────────────────────────────────
MARKER          = "<!-- changelog-suggestion-bot -->"
MAX_DIFF_CHARS  = 14_000   # keep well within model context limits
MAX_CHANGELOG_CHARS = 3_000

_TEMP = os.environ.get("RUNNER_TEMP", ".")
SYSTEM_PROMPT_FILE = os.path.join(_TEMP, "changelog_system_prompt.txt")
USER_PROMPT_FILE   = os.path.join(_TEMP, "changelog_user_prompt.txt")


# ── helpers ──────────────────────────────────────────────────────────────────

def _gh(url: str, *, method: str = "GET", data: dict | None = None, accept: str | None = None) -> object | str:
    """Minimal GitHub REST helper (no third-party deps)."""
    token = os.environ["GITHUB_TOKEN"]
    headers = {
        "Authorization": f"Bearer {token}",
        "Accept": accept or "application/vnd.github+json",
        "X-GitHub-Api-Version": "2022-11-28",
        "Content-Type": "application/json",
        "User-Agent": "inviwo/inviwo changelog-suggestion-bot",
    }
    body = json.dumps(data).encode() if data else None
    req = urllib.request.Request(url, headers=headers, method=method, data=body)
    try:
        with urllib.request.urlopen(req) as resp:
            raw = resp.read()
            content_type = resp.headers.get("Content-Type", "")
            if "json" in content_type:
                return json.loads(raw) if raw else None
            return raw.decode(errors="replace") if raw else ""
    except urllib.error.HTTPError as exc:
        print(f"GitHub API error {exc.code}: {exc.read().decode(errors='replace')}", file=sys.stderr)
        raise


def get_pr_diff(repo: str, pr_number: str) -> str:
    url = f"https://api.github.com/repos/{repo}/pulls/{pr_number}"
    diff = _gh(url, accept="application/vnd.github.v3.diff")
    text = diff if isinstance(diff, str) else ""
    if len(text) > MAX_DIFF_CHARS:
        text = text[:MAX_DIFF_CHARS] + "\n... (diff truncated for brevity)"
    return text


def find_bot_comment(repo: str, pr_number: str) -> int | None:
    url = f"https://api.github.com/repos/{repo}/issues/{pr_number}/comments?per_page=100"
    comments = _gh(url)
    for c in (comments or []):
        if MARKER in c.get("body", ""):
            return c["id"]
    return None


def upsert_comment(repo: str, pr_number: str, body: str) -> None:
    existing = find_bot_comment(repo, pr_number)
    if existing:
        url = f"https://api.github.com/repos/{repo}/issues/comments/{existing}"
        _gh(url, method="PATCH", data={"body": body})
        print(f"Updated existing bot comment #{existing}.")
    else:
        url = f"https://api.github.com/repos/{repo}/issues/{pr_number}/comments"
        _gh(url, method="POST", data={"body": body})
        print("Posted new bot comment.")


# ── subcommands ──────────────────────────────────────────────────────────────

def build_prompt() -> None:
    """Write the system and user prompt files for actions/ai-inference."""
    pr_number = os.environ["PR_NUMBER"]
    pr_title  = os.environ.get("PR_TITLE", "").strip()
    pr_body   = os.environ.get("PR_BODY", "").strip()
    repo      = os.environ["REPO"]
    today     = date.today().strftime("%Y-%m-%d")

    try:
        diff = get_pr_diff(repo, pr_number)
    except Exception as exc:  # noqa: BLE001 - never fail the check on diff issues
        print(f"Warning: could not fetch PR diff: {exc}", file=sys.stderr)
        diff = "(diff unavailable)"

    try:
        with open("CHANGELOG.md", encoding="utf-8") as fh:
            changelog_head = fh.read(MAX_CHANGELOG_CHARS)
    except FileNotFoundError:
        changelog_head = "(CHANGELOG.md not found)"

    system_prompt = f"""\
You are a technical writer maintaining the CHANGELOG.md for Inviwo, an open-source
C++/Python interactive visualization framework. The changelog documents changes that
affect the public API or that other developers need to know about.

Each entry follows this format:
  ## YYYY-MM-DD Short descriptive title
  One or more paragraphs describing the change.
  Optional subsections (### Migration guide) for breaking changes.
  Optional code blocks (C++, Python, CMake, GLSL).

Rules:
- Use today's date: {today}
- Be concise but precise; name affected classes/functions/modules/headers.
- If the change is a breaking API change add a brief "### Migration guide" subsection.
- Match the tone and style of the existing entries shown below.
- Output ONLY the raw Markdown for the new entry — no preamble, no explanation.
"""

    user_prompt = f"""\
PR title: {pr_title}

PR description:
{pr_body or "(none)"}

Code diff:
```diff
{diff}
```

Existing CHANGELOG.md (first {MAX_CHANGELOG_CHARS} chars — style reference only):
{changelog_head}

Write a suitable CHANGELOG.md entry for this PR."""

    with open(SYSTEM_PROMPT_FILE, "w", encoding="utf-8") as fh:
        fh.write(system_prompt)
    with open(USER_PROMPT_FILE, "w", encoding="utf-8") as fh:
        fh.write(user_prompt)

    print(f"Wrote system prompt to {SYSTEM_PROMPT_FILE}")
    print(f"Wrote user prompt to {USER_PROMPT_FILE}")


def post_comment() -> None:
    """Upsert the PR comment using the model response from actions/ai-inference."""
    pr_number     = os.environ["PR_NUMBER"]
    repo          = os.environ["REPO"]
    response_file = os.environ.get("RESPONSE_FILE", "").strip()

    suggestion = ""
    if response_file and os.path.isfile(response_file):
        with open(response_file, encoding="utf-8") as fh:
            suggestion = fh.read().strip()

    if not suggestion:
        print("No model response available — skipping comment.")
        return

    comment = f"""{MARKER}
## 📝 Suggested `CHANGELOG.md` entry

`CHANGELOG.md` was not updated in this PR. \
Based on the diff, here is a suggested entry — add it at the **top** of `CHANGELOG.md` if appropriate:

```markdown
{suggestion}
```

<details>
<summary>ℹ️ About this suggestion</summary>

This entry was generated automatically by analysing the PR diff with an AI model.
Please review it carefully; it may need adjustments to accurately reflect the intent of the changes.

**When a changelog entry is required:** changes that affect the public C++/Python API,
behaviour visible to other module authors, or anything that requires a migration step.

**When it can be skipped:** pure internal refactors, test-only changes, documentation
fixes, or CI tweaks. Add the `no-changelog` label to this PR to silence this bot.

</details>
"""

    upsert_comment(repo, pr_number, comment)
    print("Done ✅")


# ── main ─────────────────────────────────────────────────────────────────────

def main() -> None:
    command = sys.argv[1] if len(sys.argv) > 1 else ""
    if command == "build-prompt":
        build_prompt()
    elif command == "post-comment":
        post_comment()
    else:
        print("Usage: generate_changelog_suggestion.py {build-prompt|post-comment}", file=sys.stderr)
        sys.exit(2)


if __name__ == "__main__":
    main()
