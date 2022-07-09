# Socket Signal Chat

# Usage 
Use makefile to compile with gcc by using the command : 

```bash
make
```

# Features 

Error Handling :
- CTRL+D EOF
- Stop signals
- Many clients connected on the same pipe (set with a number of max client to modify in the `#define`) : Reject of a too big number of user wanting to connect to the same pipe
- Interface

# Used concepts : 

- fork()
- exec()
- mkfifo()
- signalHandler()
- unlink()
...

# Encountered problems : 

Lack of bidirectionnal communication. Need to be reworked.

Need to add another chat_child.c to communicate in the chat after error handling

## Credit
Seynokan