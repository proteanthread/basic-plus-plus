#!/usr/bin/env python3
# FILENAME: gate_headers.py
# LICENSE: Copyleft (c) 2026 BASIC++ Community - All Wrongs Reserved
# VERSION: 7.0.0.0
# NEEDED BY: tools/compliance/run_all.sh, CI
# NEEDS: python3 standard library only
# THE DECLARATION GATE. The 6-line file header is the only machine-readable
# statement of what a module depends on. It is worth enforcing only because
# gate_modularity.py gives it something to be checked AGAINST: a NEEDS line
# that can be compared to the measured symbol graph is a contract, one that
# cannot is a comment.
# Standard: ISO/IEC 25010:2023 Maintainability > Analysability, Modularity
#           IEEE 1016 -- a design description names its dependencies
import json, os, re, sys

CANON = {'libcore', 'libengine', 'libkernel', 'libplatform', 'libstandard',
         'libboot', 'libhardware', 'libserver', 'libreg', 'libext', 'libadvanced'}
ALIAS = {'platform': 'libplatform', 'standard': 'libstandard', 'reg': 'libreg',
         'core': 'libcore', 'engine': 'libengine', 'kernel': 'libkernel',
         'boot': 'libboot', 'hardware': 'libhardware', 'server': 'libserver'}
NEEDS = re.compile(r'^//\s*NEEDS:\s*([A-Za-z_][\w+]*)', re.M)
NEEDEDBY = re.compile(r'^//\s*NEEDED BY:\s*\S', re.M)

def main(root='.'):
    no_needs, no_by, bad_vocab = [], [], []
    for dp, _, fns in os.walk(os.path.join(root, 'engine')):
        for f in fns:
            if not f.endswith(('.c', '.h')): continue
            p = os.path.join(dp, f); rel = os.path.relpath(p, root)
            head = ''.join(open(p, encoding='utf-8', errors='replace').readlines()[:20])
            libs = NEEDS.findall(head)
            if not libs: no_needs.append(rel)
            if not NEEDEDBY.search(head): no_by.append(rel)
            for l in libs:
                if l in CANON: continue
                fix = ALIAS.get(l)
                bad_vocab.append((rel, l, fix or '(not a library name)'))
    for rel in sorted(no_needs)[:15]: print("%s: no NEEDS: line" % rel)
    for rel, l, fix in sorted(bad_vocab)[:15]:
        print("%s: NEEDS: %s -- write %s" % (rel, l, fix))
    print("GATE headers: %d file(s) with no NEEDS, %d with no NEEDED BY, "
          "%d non-canonical library name(s)" % (len(no_needs), len(no_by), len(bad_vocab)))
    base = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'baseline.json')
    b = json.load(open(base)) if os.path.exists(base) else {}
    bad = 0
    for key, val in (('headers_no_needs', len(no_needs)), ('headers_no_neededby', len(no_by)),
                     ('headers_bad_vocab', len(bad_vocab))):
        lim = b.get(key)
        if lim is not None and val > lim:
            print("  FAIL: %s regressed against baseline of %d" % (key, lim)); bad = 1
    return bad

if __name__ == '__main__':
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else '.'))
