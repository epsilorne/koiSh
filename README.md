# koiSh

A (non-POSIX) toy shell, written mainly to learn the innards of how shells work.

Other doco TBA.

## Building and Running
```bash
git clone https://github.com/epsilorne/koiSh.git
cd koiSh
make
./bin/koish
```

## Usage
koiSh is able to execute programs located in `/usr/bin/` (or the current directory using `./`).
For example:

```bash
cat src/shell.c
```

There are several builtins as well, such as `cd` or `exit`. You can see all of them using:

```bash
help
```

Programs and builtins can be executed sequentially using `;`, noting it must be separated by whitespace.
For example:

```bash
cd src ; head -n 5 shell.c
```

## Known Issues
- Piping input into `koish` breaks things
- Segfault when no. of args >= `DEFAULT_ARG_COUNT` (reallocating is broken)
- `exit` only works alone/as the last command

## TODO
- Command history
- Piping and redirecting
- Environment variables
- `.config` file (e.g. for customisable prompt and aliases)
- Autocompletion
- Anything else that might be useful for a shell
