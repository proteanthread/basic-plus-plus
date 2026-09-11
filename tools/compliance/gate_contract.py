#!/usr/bin/env python3
# FILENAME: gate_contract.py
# LICENSE: Copyleft (c) 2026 BASIC++ Community - All Wrongs Reserved
# VERSION: 7.0.0.0
# NEEDED BY: tools/compliance/run_all.sh, CI
# NEEDS: python3 standard library only
# THE CONTRACT GATE. Three ways a keyword can lie about itself:
#   ORPHAN     a LangDesc defined and never registered
#   MISFILED   one descriptor registered from two register functions, so a
#              keyword reports itself as a different keyword
#   UNDOCUMENTED  a name registered with the dispatcher and no descriptor,
#              so HELP shows nothing and the introspection JSON omits it
# Standard: ISO/IEC 25010:2023 Functional Suitability > Correctness
#           ISO/IEC/IEEE 26514 -- documentation must match the product
import collections, json, os, re, sys

DESC = re.compile(r'static\s+const\s+LangDesc\s+(\w+)\s*=\s*\{(.*?)\n\};', re.S)
NAME = re.compile(r'\.name\s*=\s*"([^"]*)"')
FNDEF = re.compile(r'\n(?:void|static\s+void)\s+(\w+)\s*\(\s*void\s*\)\s*\{(.*?)\n\}', re.S)
REG = re.compile(r'lang_desc_register\s*\(\s*&\s*(\w+)\s*\)')
SREG = re.compile(r'stmt_register\s*\(\s*[^,]+,\s*[\w]+\s*,\s*[\w]+\s*,\s*"([^"]*)"')

def main(root='.'):
    orphans, dup, descnames, regnames = [], [], {}, set()
    for dp, _, fns in os.walk(os.path.join(root, 'engine')):
        for f in fns:
            if not f.endswith('.c'): continue
            p = os.path.join(dp, f); rel = os.path.relpath(p, root)
            s = open(p, encoding='utf-8', errors='replace').read()
            regnames |= {m.group(1).upper() for m in SREG.finditer(s)}
            descs = {}
            for m in DESC.finditer(s):
                nm = NAME.search(m.group(2))
                descs[m.group(1)] = (nm.group(1) if nm else '?', 'FEATURE_STATEMENT' in m.group(2))
            if not descs: continue
            used = collections.defaultdict(list)
            for fm in FNDEF.finditer(s):
                for rm in REG.finditer(fm.group(2)): used[rm.group(1)].append(fm.group(1))
            allrefs = set(REG.findall(s))
            for var, (nm, is_stmt) in descs.items():
                if is_stmt: descnames[nm.upper()] = rel
                if var not in allrefs: orphans.append((rel, var, nm))
            for var, fs in used.items():
                if len(set(fs)) > 1:
                    dup.append((rel, var, descs.get(var, ('?', 0))[0], sorted(set(fs))))
    undoc = sorted(regnames - set(descnames))
    for rel, var, nm in sorted(orphans):
        print("%s: %s (\"%s\") is defined and never registered" % (rel, var, nm))
    for rel, var, nm, fs in sorted(dup):
        print("%s: %s (\"%s\") registered from %s" % (rel, var, nm, ", ".join(fs)))
    print("GATE contract: %d orphan descriptor(s), %d misfiled, %d registered names "
          "with no descriptor" % (len(orphans), len(dup), len(undoc)))
    base = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'baseline.json')
    b = json.load(open(base)) if os.path.exists(base) else {}
    bad = 0
    for key, val in (('contract_orphans', len(orphans)), ('contract_misfiled', len(dup)),
                     ('contract_undocumented', len(undoc))):
        lim = b.get(key)
        if lim is not None and val > lim:
            print("  FAIL: %s regressed against baseline of %d" % (key, lim)); bad = 1
    return bad

if __name__ == '__main__':
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else '.'))
