# pg_hook_minimal

This repo shows an example of how to create a minimal Postgres hook for prototyping and points to some resources/docs on hooks. The example given is for the query optimizer hook (`planner_hook`), but most hooks will have a similar file/code structure so this repo should be a good starting point for any hook.

*WARNING* I put this together during a hack week so I haven't tested actually getting a hook deployed, this example is only for prototyping purposes at this time.

## Choose a Hook (or Multiple Hooks)

The ways I find available hooks are:

1. Read these unofficial hook docs: https://github.com/taminomara/psql-hooks.
2. Search the [contrib](https://github.com/postgres/postgres/tree/master/contrib) directory in the postgresql codebase for `_hook`.
3. Search the whole postgresql [codebase](https://github.com/postgres/postgres/tree/master) for `_hook` or the relevant component of interest to you.

## Development

Below are some notes about you can run Postgres locally on an Ubuntu VM, develop, and debug it.

### Build Postgres in an Ubuntu VM via Multipass

[Multipass Docs](https://multipass.run), you'll need to install multipass.


Start multipass and install dev deps.
```shell
# After launching the VM, you can just run `multipass shell` to reconnect
multipass launch 22.04 -n yourvmnamehere -c 4 -m 8G -d 50G

multipass shell yourvmnamehere

# https://www.postgresql.org/docs/current/install-requirements.html
sudo apt update
sudo apt install gcc g++ libreadline-dev zlib1g zlib1g-dev bison flex make gdb pkg-config libicu-dev -y
```

```shell
# clone postgresql (I used commit `b7f315c9d7d839dda847b10d170ffec7c3f4dbba` from `Fri Nov 10 22:46:46 2023 +0200`)
git clone https://git.postgresql.org/git/postgresql.git
cd postgresql
```

```shell
# build and install postgresql
# https://www.postgresql.org/docs/current/installation.html
./configure --enable-cassert --enable-debug CFLAGS="-ggdb -Og -g3 -fno-omit-frame-pointer"
make
make check
sudo make install

# setup/start postgres and create a database
mkdir -p ~/pgdata
/usr/local/pgsql/bin/initdb -D ~/pgdata
/usr/local/pgsql/bin/pg_ctl -D ~/pgdata -l logfile start
/usr/local/pgsql/bin/createdb foo
```

You've got PostgreSQL running in an Ubuntu VM.

### Build and Install your Hook

Now we're going to clone this minimal hook and get it installed into PostgreSQL.

Note that if you're not very comfortable with C/Makefiles/etc, you may want to get started by getting the commands below working without changing any names and using the default hook name of `pg_hook_minimal`, then you can go back and change the names per the instructions below.

```
# Open a new terminal and get a shell into the VM
multipass shell yourvmnamehere
git clone https://github.com/nathanwilk7/pg_hook_minimal.git your_hook_name_goes_here
cd your_hook_name_goes_here
```

- In the [Makefile](Makefile), replace `pg_hook_minimal` with whatever name you want for your hook. See [Hooks in PostgreSQL](https://wiki.postgresql.org/images/e/e3/Hooks_in_postgresql.pdf), page 33 (Compiling Hooks).
- Rename [pg_hook_minimal.control](pg_hook_minimal.control) with the name of your hook and replace `pg_hook_minimal` in the file with your hook name. Taken from [pgtam/pgtam.control](https://github.com/eatonphil/pgtam/blob/main/pgtam.control). NOTE verify this is needed.
- Add your hook to `~/pgdata/postgresql.conf` by searching for `shared_preload_libraries` and replacing it with the below. While developing, you'll have to restart the server for it to to pickup changes you make (TODO find a way to hot reload the extension). See [Hooks in PostgreSQL](https://wiki.postgresql.org/images/e/e3/Hooks_in_postgresql.pdf), page 36 (Using hooks).
```
shared_preload_libraries = 'your_hook_name_here'      # (change requires restart)
```
- Rename [pg_hook_minimal.c](pg_hook_minimal.c) with the name of your hook and replace all the references to `plan` with names/identifiers for the hook you've chosen to override. You'll have to change the function signatures/types as well.

Compile and install the hook:
```
make USE_PGXS=1 && sudo make USE_PGXS=1 install
```

Restart the postgres server and check for the loaded hook log:
```
/usr/local/pgsql/bin/pg_ctl -D ~/pgdata -l logfile restart
# check that `LOG:  Loaded hook pg_foo ...` was logged
cat ~/postgresql/logfile
```

```
# Open a new terminal and get a shell into the VM
multipass shell yourvmnamehere

# Get a psql prompt
/usr/local/pgsql/bin/psql foo

# Run a simple select, you should see `NOTICE:  Running hook ...` above the result
select 1;
```

### GDB + psql Debug Workflow

Terminal 1
- Edit the `your_hook_name.c` file (e.g.: `vi your`)
- Save the file and run `make USE_PGXS=1 && sudo make USE_PGXS=1 install`

Terminal 2
- Run `/usr/local/pgsql/bin/pg_ctl -D ~/pgdata -l logfile restart`

Terminal 3 (after first time, you can leave this running and reconnect after restarting the server)
- Run `/usr/local/pgsql/bin/psql foo`

Terminal 4 (Optional, to set a gdb breakpoint in your hook function, replace `your_hook_name.c` with the filename you chose and `your_hook_name` with your hook function name)
- Run `sudo gdb -p $(pgrep -f "postgres.*local") -ex 'b your_hook_name.c:your_hook_name'`

Terminal 3
- Run SQL/commands of interest (e.g.: `select 1` or `select * from foo`)

Repeat.

## Other Resources

- [Hooks in PostgreSQL](https://wiki.postgresql.org/images/e/e3/Hooks_in_postgresql.pdf)
- [Getting a hook on PostgreSQL extensibility](https://archive.fosdem.org/2021/schedule/event/postgresql_extensibility/attachments/slides/4348/export/events/attachments/postgresql_extensibility/slides/4348/fosdem21_postgres_extensibility.pdf)
- [Using PostgreSQL Hooks](https://www.endpointdev.com/blog/2010/05/using-postgresql-hooks/)
- [Babelfish PostreSQL Hooks](https://babelfishpg.org/docs/internals/postgresql-hooks/)
- [Hooks: The secret feature powering the Postgres ecosystem](https://devpress.csdn.net/postgresql/62f4d9037e66823466188f42.html)
- [pg_hint_plan](https://github.com/ossc-db/pg_hint_plan)
- [pg_tle](https://github.com/aws/pg_tle/tree/main)
- [pg_ivm](https://github.com/sraoss/pg_ivm)

## Future

- One liner to easily replace hook name with your own (or template/cookiecutter)
- Testing
- Hot reloading of hooks
- Local filesystem/IDE integration
- Distribution
