# pg_hook_minimal

This repo shows an example of how to create a minimal Postgres hook for prototyping and points to some resources/docs on hooks. The example given is for the query optimizer hook (`planner_hook`), but most hooks will have a similar file/code structure so this repo should be a good starting point

*WARNING* I put this together during a hack week so I haven't tested actually getting a hook deployed, this example is only for prototyping purposes at this time.

## Setup

- In the [Makefile](Makefile), replace `pg_hook_minimal` with the name of your hook. See [Hooks in PostgreSQL](https://wiki.postgresql.org/images/e/e3/Hooks_in_postgresql.pdf), page 33 (Compiling Hooks).
- Rename [pg_hook_minimal.control](pg_hook_minimal.control) to have the name of your hook and replace `pg_hook_minimal` in the file with your hook name. Taken from [pgtam/pgtam.control](https://github.com/eatonphil/pgtam/blob/main/pgtam.control). NOTE verify this is needed.
- Add your hook to `postgresql.conf`. You'll have to restart the server for it to to pickup changes you make (TODO find a way to hot reload the extension).
```
shared_preload_libraries = 'pgplannerhook'      # (change requires restart)
```

## Development

```
make USE_PGXS=1 && sudo make USE_PGXS=1 install
```

## Notes

Below are some notes about running Postgres locally and debugging it.
