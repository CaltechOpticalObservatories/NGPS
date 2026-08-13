# Command Line Interface

```{note}
This is a future feature.
```

A command line interface gives users the ability to send interactive commands and to create scripts to control any aspect of the instrument. A single wrapper, `seq`, can be entered into any shell and provides the interface for both scripts and single interactive commands. The general syntax structure is:

```text
seq <command>
seq <command> <key>=<value>
seq <command> <key1>=<value1> <key2>=<value2> ...
```

Scripts can be made using any plain text editor and have no restrictions on the file name. Scripting syntax uses the following rules:

```text
# comments begin with '#'
# one command per line
command
command key=value
command key1=value1 key2=value2

# execution grouping keywords
parallel:
serial:
end
on_error stop|continue

# rules
- commands require a group start and end
- 'serial:' group start is optional (default is serial) but when used,
  requires a matching 'end'
- no nested groups
- 'end' must have a matching group start, serial: or parallel:
- 'on_error' sets the behavior when an error occurs in a group,
  - stop means to stop execution of group when an error occurs
  - continue means to continue to the next command if an error occurs
- all other first tokens must be valid commands
- unknown commands/keywords are errors
- malformed key=value pairs are errors
- empty groups are errors
- blank lines are ignored
- comments are ignored

# example
parallel:
move_to_target ra=12:34:56.7 dec=+12:34:56.7
set_slit width=0.5
set_camera exptime=0
end
expose
set_camera binspat=2 binspec=3
expose
```

The command to run a script is `run`, for example:

```text
seq run myscript.seq
```

Note that the `.seq` extension is not a requirement, only an example. A complete list of commands is given in the {doc}`detector-and-command-reference`.

```{note}
Not yet implemented.
```
