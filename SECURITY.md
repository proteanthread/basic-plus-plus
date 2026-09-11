# Security Policy

BASIC++ takes security reports seriously and will respond to every one. This
document says how to report a vulnerability, what happens next, and what is
currently known to be insecure.

---

## Reporting a vulnerability or a safety defect

**Do not open a public issue.** Public issues are visible to everyone the
moment they are filed, including to anyone who would rather exploit the defect
than see it fixed.

**Use GitHub's private vulnerability reporting.** Go to the
[Security tab of this repository](https://github.com/proteanthread/basic-plus-plus/security)
and press **Report a vulnerability**. The report goes straight to the
maintainers as a private draft security advisory. It is not public, it is not
indexed, and no email address is exchanged in either direction — GitHub carries
the whole conversation, including the fix and the eventual disclosure, in one
place.

You need a GitHub account to use it. That is the only requirement.

### What counts

Report anything in either category:

**Security** — memory corruption, buffer overruns, use-after-free, sandbox or
capability escapes, path traversal out of a mounted filesystem, anything that
lets an untrusted BASIC program reach the host, and any weakness in the
cryptographic, authentication or authorisation paths.

**Safety** — a defect that could cause harm through incorrect behaviour rather
than through attack. Silent wrong results, a hardware access that appears to
succeed and does nothing, unbounded loops or allocations, and non-deterministic
timing in code that is documented as deterministic. This project targets
firmware, boot code and embedded devices, so a silent wrong answer is a safety
matter and not only a correctness one.

If you are unsure which it is, report it anyway and let us classify it.

### What to include

As far as you can:

- The version or commit hash affected
- The build target: `baspp`, `bpp`, `bs`, `bppc`, `trans`, `iot`, the
  `basicpp` shared library, or the WebAssembly target
- The platform, architecture and compiler
- A minimal BASIC program or C harness that reproduces it
- What you expected to happen and what happened instead
- Whether you believe it is being exploited

You do not need a working exploit. A clear description of the defect is enough.

## What happens next

| When | What |
|---|---|
| Within 48 hours | Acknowledgement that the report was received |
| Within 7 days | An initial assessment: confirmed, not reproducible, or not a vulnerability, with reasoning |
| Within 90 days | A fix released, or an explanation of why it is taking longer and a revised date |

If a report concerns a vulnerability that is being actively exploited, the
timeline compresses: acknowledgement within 24 hours and a status update
within 72 hours.

We will keep you informed at each step in the advisory thread, and we will
credit you in the release notes and in the published advisory unless you ask us
not to.

---

## Coordinated disclosure

We ask for 90 days between report and public disclosure, and we will usually
be faster. If you intend to disclose earlier, tell us when, and we will work
to that date rather than argue about it.

We will not take legal action against anyone who reports a vulnerability in
good faith, follows this policy, and does not access, modify or destroy data
belonging to others while researching.

---

## Supported versions

| Version | Supported |
|---|---|
| 6.5.x | Yes |
| 6.4.x and earlier | No |

**Security support period:** each minor release is supported for security
fixes for **[N] years** from its release date.

> This figure must be set before publishing. The EU Cyber Resilience Act's
> expectation for a product's support period is a minimum of five years, and
> a stated period is itself an Annex I requirement.

---

## Known security limitations in the current release

Stating these openly is deliberate. Software that hides its weak points is
harder to use safely than software that names them.

### Cryptography — do not rely on it

`CRYPTO.ENCRYPT$` and `CRYPTO.DECRYPT$` are implemented in
`engine/src/runtime/crypto_engine.c` as `crypto_encrypt_sim` and
`crypto_decrypt_sim`. The algorithm is a byte-wise XOR against a repeating
key. That is a Vigenère cipher, breakable by hand, and it provides no
confidentiality against anyone.

`CRYPTO.KEY$` calls `crypto_keygen`, which computes each byte as
`(i * 37 + 101) ^ (bytes * 13)`. There is no entropy source. The same call
returns the same bytes on every machine, every time. Anyone can compute the
"key".

**Do not use these for anything that matters.** Both are scheduled for
replacement in 7.0.0 as the first item of work in the release.

`CRYPTO.HASH$` and `CRYPTO.HMAC$` are different and are safe to rely on for
integrity checking: `crypto_sha256` is a genuine SHA-256 with the correct
initial hash values, and `crypto_hmac_sha256_hex` is a real HMAC. Pass the
algorithm name `"SHA256"` explicitly — an unrecognised name currently falls
back to FNV-64, which is not a cryptographic hash. That fallback is also
being fixed.

### No transport security

There is no TLS anywhere in the project. No connection BASIC++ makes is
encrypted or authenticated, and no `https://` or `gemini://` URL can be
fetched honestly. Do not send credentials or sensitive data over any
BASIC++ network facility.

### Network keywords do not reach the network

The `SOCK.*` keywords, the `TCP:` device, `HTTP.GET$`, and the `WIFI`, `MQTT`,
`ESPNOW`, `BT` and `WEBREPL` families currently report success without
performing any network operation. This is a correctness problem rather than a
vulnerability, but it has a security consequence worth stating: **a program
that appears to have verified something over the network has verified
nothing.** Do not use them in any security-relevant check.

### Port and memory access on non-x86 targets

`OUT`, `INP`, `PEEK` and `POKE` are implemented for x86 and x86_64. On any
other architecture the port functions compile to a no-op returning zero, with
no diagnostic. A program that believes it has written to a hardware register
on ARM or RISC-V has not.

### The sandbox is advisory — do not run untrusted programs

An earlier version of this document said the safety model was "enforced
rather than advisory". That was wrong, and the audit that found it is
recorded as AUD-0149 in the project's defect register.

What exists: a global security level (`SECURITY OPEN` through
`SECURITY PARANOID`, default **OPEN**), a per-keyword restriction list set by
`SCOPE DISABLE`, a required level on module specs, and one capability check on
file channels. The per-keyword `SAFETY_*` level carried by every descriptor is
metadata only — nothing reads it.

What that means in practice, verified on the current release:

```
10 SECURITY PARANOID
20 EXTERN "libc.so.6", "getpid"           ' accepted: native code loads
30 SHELL "echo shell-ran"                 ' runs
40 OPEN "/tmp/x" FOR OUTPUT AS #1         ' file written
```

`EXTERN`, `SHELL` and `OPEN` do not consult the security level at all. The
statements that do — sound, audio, task and introspection — are the least
dangerous in the language.

**Do not run BASIC programs you do not trust, at any security level, until
this is fixed.** The fix is scheduled as the first item of the 7.0.0 build:
make the dispatcher read the safety level it already carries, and gate
`EXTERN`, `SHELL`, `OPEN` and the network family on it.

---

## Scope

In scope: the engine, the standard library, the compiler and transpiler
(`bppc`, `trans`), the shared library and its C API, the WebAssembly target,
and the build system.

Out of scope: the bundled third-party components (SDL2, TinyGL), and the
`source/` and `v5/` legacy trees, which are not built and are being removed.

---

## Regulatory context

This policy exists in part to satisfy Annex I Part II of Regulation (EU)
2024/2847, the Cyber Resilience Act, which requires a coordinated
vulnerability disclosure policy, a documented process for identifying,
remediating and disclosing vulnerabilities, and reporting of actively
exploited vulnerabilities and severe incidents.

Where the Act's reporting obligations apply, the timelines are 24 hours for
an initial alert from discovery, 72 hours for a detailed notification
including corrective actions, and 14 days for a final report once resolved.

Whether those obligations bind this project depends on how it is distributed
and monetised. That determination is recorded in `PROJECT_RULES.md`.

---

## For maintainers

Private vulnerability reporting must be switched on for the button above to
exist. Enable it once, at:

> **Settings → Security and quality → Private vulnerability reporting → Enable**

Until it is enabled, a researcher following this document finds no way to
report privately, and the most likely outcome is a public issue disclosing the
defect to everyone at once.

---

*Last reviewed: 2026-09-11*
