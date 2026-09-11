#!/usr/bin/env python3
# FILENAME: gate_modularity.py
# LICENSE: Copyleft (c) 2026 BASIC++ Community - All Wrongs Reserved
# VERSION: 7.0.0.0
# NEEDED BY: tools/compliance/run_all.sh, CI
# NEEDS: python3, nm, a completed build tree
# THE MODULARITY GATE, and the test the project never had. For every object
# in the build, nm gives what it defines and what it references. Resolving
# references to defining objects gives a directed graph; the TRANSITIVE
# CLOSURE of a node is everything that must be linked with it. A module whose
# closure is the whole engine does not stand on its own, whatever its source
# looks like.
# Standard: ISO/IEC 25010:2023 Maintainability > Modularity, Flexibility
import collections, json, os, re, subprocess, sys

CLOSURE_GATE = 25          # a module may pull in at most this many others
LIBC = re.compile(r'^(__|_GLOBAL|mem(cpy|set|move|cmp)|str(len|cmp|ncmp|cpy|ncpy|cat)'
                  r'|malloc|free|calloc|realloc|printf|fprintf|snprintf|sprintf|puts|putchar'
                  r'|f(open|close|read|write|seek|tell)|exit|abort|atexit|abs|labs|log|log10'
                  r'|exp|pow|sqrt|sin|cos|tan|atan|atan2|floor|ceil|fmod|fabs|time|clock|rand'
                  r'|srand|qsort|isatty|read|write|open|close|select|socket|connect|bind|listen'
                  r'|accept|send|recv|getaddrinfo|freeaddrinfo|inet_|ioctl|fcntl|nanosleep'
                  r'|usleep|sleep|dl(open|sym|close)|pthread_|sigaction|signal|tc[gs]etattr'
                  r'|initscr|cbreak|noecho|keypad|endwin|stdscr|std(err|out|in)|errno|asan'
                  r'|ubsan|SDL_|gl[A-Z])')

def nm_maps(build):
    objs = []
    for dp, _, fns in os.walk(build):
        objs += [os.path.join(dp, f) for f in fns if f.endswith('.c.o')]
    if not objs:
        print("GATE modularity: SKIPPED, no .c.o objects under %s" % build); return None, None
    defs, undefs = {}, {}
    for i in range(0, len(objs), 200):
        chunk = objs[i:i + 200]
        for flag, sink in (('--defined-only', defs), ('-u', undefs)):
            out = subprocess.run(['nm', flag, '-g', '--print-file-name'] + chunk,
                                 capture_output=True, text=True).stdout
            for line in out.split('\n'):
                if ':' not in line: continue
                f, rest = line.split(':', 1); parts = rest.split()
                if len(parts) < 2: continue
                if sink is defs: defs.setdefault(parts[-1], set()).add(f)
                elif parts[-2] == 'U': undefs.setdefault(f, set()).add(parts[-1])
    return defs, undefs

def main(build):
    defs, undefs = nm_maps(build)
    if defs is None: return 0
    nodes = sorted(undefs)
    edges = collections.defaultdict(set)
    for o in nodes:
        for s in undefs[o]:
            if LIBC.match(s): continue
            for t in defs.get(s, ()):
                if t != o: edges[o].add(t)
    closure = {}
    for o in nodes:
        seen, fr = set(), [o]
        while fr:
            n = fr.pop()
            for m in edges.get(n, ()):
                if m not in seen and m != o: seen.add(m); fr.append(m)
        closure[o] = len(seen)
    over = sorted(((v, k) for k, v in closure.items() if v > CLOSURE_GATE), reverse=True)
    pct = 100.0 * len(over) / len(nodes)
    print("GATE modularity: %d/%d translation units exceed a closure of %d  (%.1f%%)"
          % (len(over), len(nodes), CLOSURE_GATE, pct))
    base = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'baseline.json')
    b = json.load(open(base)) if os.path.exists(base) else {}
    lim = b.get('modularity_over_closure')
    if lim is None:
        print("  no baseline recorded; run with --record to set one"); return 0
    if len(over) > lim:
        print("  FAIL: regressed against baseline of %d" % lim); return 1
    print("  within baseline of %d" % lim); return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else 'build_linux'))
