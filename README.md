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