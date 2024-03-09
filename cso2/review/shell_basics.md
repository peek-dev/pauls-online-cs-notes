
**Note:** this guide assumes you are running an instance of Bash or zsh (which is a compatible superset of Bash). Using other shells (especially PowerShell) can introduce compatibility issues and hinder your ability to follow along with these commands. 
* Bash is installed on most versions of Linux as the default shell.
* zsh is installed on many of the newest versions of MacOS (and a few Linux distributions) by default. It's slightly fancier than Bash and, as a superset, has all the Bash essentials needed for this guide.

A sample Bash terminal ("terminal session") will likely look similar to the following by default:
```sh
pkarhnak@KATHERINE:~$ 
```

A sample zsh session will likely look similar to the following by default.
```sh
pkarhnak@KATHERINE ~
%
```

The `$` or `%` doesn't really matter. In fact, this can be customized and changed from the default. Such a symbol is known as the **prompt character** since this distinguishes shell information (the user, hostname, and current directory) from a command. 

# Hello, World! and the parts of a command

Here is a "Hello, world!" example in Bash and zsh. You'll notice that only the presentation differs; `echo` is a common _utility_ (standard program) that is shared on systems which use both Bash and zsh. In fact, you could even run this command on Windows PowerShell, and it will do the same thing.

`echo` is a simple command-line program that displays a line of text to the terminal (what will be referred to throughout as "standard output" when visible to the user). This is much like if you were to run `print()` in Python. If we forget what `echo` does, we can always refresh ourselves by looking at the manual page with `man echo`.

Given that, here's "Hello, world!" in Bash:

```sh
pkarhnak@KATHERINE:~$ echo 'Hello, world!'
Hello, world!
```

Here's "Hello, world!" in zsh:

```sh
pkarhnak@KATHERINE ~
% echo 'Hello, world!'
Hello, world!
```
In each example, our **user** is `pkarhnak`, our **hostname** is `KATHERINE`, our **current directory** is `~` (a special character equivalent to the _home directory_, or the directory we are placed into by default when logging in to the shell), and we are running the `echo` command with **argument** `'Hello, world'`. The first two of these will show up for all the commands we run except if we change which user we are, if we run a shell on a different host, or both. The third will change depending on which directory we are running in (which we can change with `cd`). The command we run is up to us; arguments to that command are contextual, meaning that what arguments do and the accepted list of arguments will vary between `echo` and other commands like `cd`, `ls`[^1], and `pwd`[^2]).

Note that the `Hello, world!` line that is last in each of the two above code blocks is what is printed _for us_ by running `echo`.

[^1]: To **l**i**s**t the contents of a directory.
[^2]: To **p**rint the **w**orking (current) **d**irectory 
