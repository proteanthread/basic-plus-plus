# Project Gemini & RFC 1436 Gopher Protocols

BASIC++ includes native client and server micro-libraries for lightweight web protocols: Gemini (RFC-style TLS encapsulation) and Gopher (RFC 1436).

## 1. Built-in Retrieval Functions
- `GEMINI.GET$(url$)`: Fetches a Gemini capsule document (`gemini://...`) and returns its Gemtext (`text/gemini`) body string.
- `GOPHER.GET$(url$)`: Fetches an RFC 1436 Gopher menu or document (`gopher://...`) and returns its plaintext tab-delimited representation.

## 2. Embedded Protocol Servers
- `GEMINI.SERVE [port [, root_dir$]]`: Spawns a background Gemini capsule server (default port 1965).
- `GOPHER.SERVE [port [, root_dir$]]`: Spawns a background Gopher server (default port 70).

## 3. FujiNet Virtual Bus Device `N:`
- `OPEN "N:GEMINI://host/path" FOR INPUT AS #ch`: Streams Gemtext lines via standard `LINE INPUT #ch, L$` and `INPUT #ch`.
- `OPEN "N:GOPHER://host/path" FOR INPUT AS #ch`: Streams Gopher menus through the retro peripheral bridge.
