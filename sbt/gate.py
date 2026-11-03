"""Pass or fail for a test bed. A bed exits 1 unless every case passes or is a known failure listed with its reason
(docs/limits.md), and a run that timed out never counts as agreeing with another run, even one that timed out too."""
###############
# imports and constants
###############
from __future__ import annotations
import sys

TIMEOUT = -999                                                     # exit status the runners record for a timed-out run
TIMEOUT_TEXT = "TIMEOUT\n"                                         # how stdout+exit text runners end a timed-out run
PASS = {"MATCH", "LOUD-OK", "LIMIT", "LIMIT-FIXED", "PASS", "OK"}  # LIMIT: a listed limit that shows (docs/limits.md)

###############
# functions
###############
def same(got: tuple | str, want: tuple | str) -> bool:
    """True when two runs agree and neither timed out: (exit, stdout, ...) tuples, or stdout+exit texts."""
    if isinstance(got, str): return got == want and not got.endswith(TIMEOUT_TEXT)
    return TIMEOUT not in (got[0], want[0]) and (got[0], got[1]) == (want[0], want[1])

def finish(verdicts: list[tuple[str, str]], known: dict[str, str] | None = None) -> None:
    """Exit 1 if a case's verdict word is not in PASS and the case is not in known (case -> reason), else 0, after one gate line."""
    known = known or {}
    word = {case: (v.split() or [""])[0] for case, v in verdicts}
    bad = [c for c, w in word.items() if w not in PASS and c not in known]
    held = [c for c, w in word.items() if w not in PASS and c in known]
    fixed = [c for c, w in word.items() if w in PASS and c in known]
    print("gate: %s, %d/%d pass%s%s%s" % ("FAIL" if bad or not word else "PASS", len(word) - len(bad) - len(held), len(word),
          "; failing: %s" % bad if bad else "", "; known failures: %s" % {c: known[c] for c in held} if held else "",
          "; known failures that now pass (drop them from the list): %s" % fixed if fixed else ""), flush=True)
    sys.exit(1 if bad or not word else 0)
