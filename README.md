# koiSh

A (non-POSIX) toy shell, written mainly to learn the innards of how shells work.

Other doco TBA.

## Installation and Running
```bash
git clone https://github.com/epsilorne/koiSh.git
cd koiSh
make
./bin/koish
```

## Known Issues
- Piping input into `koish` breaks things
- Segfault when using builtins
- Segfault when no. of args >= `DEFAULT_ARG_COUNT` (reallocating is broken)

## TODO
- Command history
- Piping and redirecting
- Environment variables
- `.config` file (e.g. for customisable prompt and aliases)
- Autocompletion
- Anything else that might be useful for a shell
