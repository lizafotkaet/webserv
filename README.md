# webserv
погнали

## Mika's Ramblings

AI-generated table to help me keep track of what needs to be parsed in a certain way:
# webserv config directives

| Directive | Where | Repeatable | Value format |
|---|---|---|---|
| `server` | top level | yes | `server { ... }` |
| `location` | server | yes, but each path only once | `location <path> { ... }` |
| `listen` | server | yes | `<port>` or `<ip>:<port>` |
| `server_name` | server | no | `<name> [<name> ...]` |
| `error_page` | server | yes | `<code> [<code> ...] <path>` |
| `client_max_body_size` | server, location | no | `<number>[K\|M\|G]` |
| `root` | server, location | no | `<path>` |
| `index` | server, location | no | `<file>` |
| `autoindex` | server, location | no | `on` \| `off` |
| `allow_methods` | location | no | `<method> [<method> ...]`, from GET, POST, DELETE |
| `return` | location | no | `<code> <url>` |
| `upload_store` | location | no | `<path>` |
| `cgi` | location | yes, but each extension only once | `<.ext> <interpreter path>` |

## Notes

- Directives allowed in both server and location are inherited: a location that doesn't set one takes the server's value.
- The same `listen` value may appear in different server blocks (virtual hosting, separated by `server_name`), but not twice in one server.
- The first server for a given host:port is the default for that address.
- `error_page` codes should be 300–599; `return` codes should be 3xx.
- `listen` without an IP binds to `0.0.0.0`. Port must be 1–65535.
- Duplicate non-repeatable directives in the same block are a config error.

## Notation

- `<name>`: placeholder for a value
- `[ ... ]`: optional part
- `...`: the previous item can repeat
- `a | b`: one of the listed options

Below is yet another AI generated table of headers that webserv needs to support and how to store them.
This is useful for me during HTTP parsing mostly
## Request headers

Every header is stored in a generic map: names lowercased, repeated values comma-joined. CGI receives all of them as `HTTP_*` environment variables. The headers below are the ones webserv actively acts on.

| Header | Why webserv needs it | If it appears twice | How to store it |
|---|---|---|---|
| `Host` | Required in HTTP/1.1 (400 if missing). It picks the server block via `server_name` | Reject with **400** | Hostname lowercased, with the port split off and stored separately. The raw value is still kept for CGI |
| `Content-Length` | Tells you where the body ends. Also checked against `client_max_body_size` (413 if too big) | Reject with **400** | Unsigned integer. Digits only, and check for overflow. Anything else is a 400 |
| `Transfer-Encoding` | Chunked request bodies, which the subject requires you to unchunk before passing them to CGI | Comma-join | In practice a single "is chunked" flag. If the last coding isn't `chunked`, reject. Unsupported codings get a **501** |
| `Content-Type` | Passed to CGI as `CONTENT_TYPE`. Uploads need its `boundary` parameter to split a `multipart/form-data` body | Reject with **400** | String. Parse out the media type and parameters (like `boundary`) when you need them |

### Rules across headers

- **`Content-Length` and `Transfer-Encoding` both present:** reject with 400.
- **Body expected but neither header present:** 411 Length Required.
- **Header names:** compare them case-insensitively. Lowercase them on insert so lookups are simple.
- **`Expect: 100-continue`** is optional. If you ignore it, curl waits about a second and then sends the body anyway. If you want, you can reply with `100 Continue` first.