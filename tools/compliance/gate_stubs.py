#!/usr/bin/env python3
# FILENAME: gate_stubs.py
# LICENSE: Copyleft (c) 2026 BASIC++ Community - All Wrongs Reserved
# VERSION: 7.0.0.0
# NEEDED BY: tools/compliance/run_all.sh, CI
# NEEDS: python3 standard library only
# THE SAFETY GATE. A handler whose whole body is
#   BppError err; runtime_memset(&err,0,sizeof(err)); (void)vm; (void)lex;
#   return err;
# returns a ZEROED error, which is SUCCESS. The caller cannot tell that
# nothing happened. An unimplemented layer must FAIL HONESTLY -- return an
# error sentinel, as engine/src/hal/hal_freestanding.c does throughout.
# Standard: ISO/IEC 25010:2023 Safety, Reliability > Fault Tolerance
#           Directive (EU) 2024/2853 -- a silent wrong result is the exposure
import os, re, sys, json

FN = re.compile(r'\n(?:static\s+)?(BppError|BValue|bool|int|void|double)\s+(\w+)\s*'
                r'\(([^;{)]*)\)\s*\{(.*?)\n\}', re.S)
OKLINE = [re.compile(p) for p in (
    r'BppError\s+err\s*;$', r'runtime_memset\(&err,\s*0,\s*sizeof\(err\)\);$',
    r'BppError\s+err\s*=\s*\{\s*0\s*\}\s*;$', r'(\(void\)\s*\w+\s*;\s*)+$')]
RET_SUCCESS = re.compile(r'return\s+(err|true|0)\s*;$')

def main(root='.'):
    hits = []
    for dp, _, fns in os.walk(os.path.join(root, 'engine')):
        for f in fns:
            if not f.endswith('.c'): continue
            p = os.path.join(dp, f); rel = os.path.relpath(p, root)
            s = open(p, encoding='utf-8', errors='replace').read()
            for m in FN.finditer(s):
                rt, name, params, body = m.groups()
                b = re.sub(r'/\*.*?\*/', '', re.sub(r'//[^\n]*', '', body), flags=re.S)
                lines = [l.strip() for l in b.split('\n') if l.strip()]
                if not lines: continue
                ok, success = True, False
                for l in lines:
                    if any(r.fullmatch(l) for r in OKLINE): continue
                    if RET_SUCCESS.fullmatch(l): success = True; continue
                    if re.fullmatch(r'return\s+(false|-1|NULL|0\.0)\s*;', l): continue
                    ok = False; break
                nparams = 0 if params.strip() in ('', 'void') else params.count(',') + 1
                if ok and success and nparams > 0 and name.startswith('stmt_') \
                   and name.endswith('_handler'):
                    hits.append((rel, s[:m.start()].count('\n') + 2, name))
    for rel, ln, name in sorted(hits)[:30]:
        print("%s:%d: %s does nothing and returns SUCCESS" % (rel, ln, name))
    print("GATE stubs: %d statement handler(s) report success without acting" % len(hits))
    base = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'baseline.json')
    b = json.load(open(base)) if os.path.exists(base) else {}
    lim = b.get('stub_handlers')
    if lim is None: return 0
    if len(hits) > lim:
        print("  FAIL: regressed against baseline of %d" % lim); return 1
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else '.'))
