Hardware-legible permissions are quite simple and mainly answer the question of "who has privilege to access certain sensitive system resources?" On top of this, the operating system may (and in many cases does) implement a more comprehensive permissions structure as it wishes.

# Per-process permissions

When opening a file (e.g., with `open("file", O_RDONLY)`, a C system call wrapper), the function you call does very minimal setup before passing the baton to a system call. The system call handler itself handles whether you should be able to open the file and will do this by comparing:
    * The permissions that the process which attempts the access has
    * The restrictions on the requested resource (e.g., a file)

To facilitate this, the OS maintains per-process information related to processor state, page table base address, open file descriptors, process name, total runtime, and scheduling priority. Most OSes also maintain the user account which a process is running under.

# User accounts

User accounts are a common operating system feature which wrap the following information:
* Login name (e.g., `user`)
* Real name (e.g., `Use R. Name`)
* Home directory (e.g., `/home/user` on Unix-like systems) which stores specific per-uses preferences and configurations
* A password's cryptographic hash, stored instead of plaintext so that a password's plaintext cannot be directly compromised
* A set of groups the user belongs to (a unique integer and string)

Permissions aren't generally listed per-user; rather, they're listed per-resource. On POSIX-like systems, these are implemented for owning user/owning group/world ("u/g/o") categories. 

## File permissions example

We can take a look at an example POSIX file permissions listing that could be output by `ls -l` or a similar utility:

```
-rwxr-x--- 4 user users_group	371 Sep 4  2023 file.txt
```
* The leading `-` indicates a normal file (not a directory)
* `rwx`: the owner (which is listed as `user`) is allowed to **r**ead, **w**rite, and e**x**ecute this file
* `r-x`: other users in the owning group (members of `users_group` besides `user`) may **r**ead or e**x**ecute this file
* `---`: the world (users that are not `user` and not in the group `users_group`) may neither **r**ead, nor **w**rite, nor e**x**ecute this file
* `user`: the owning user of the file, who is named `user` in this example
* `users_group`: the owning group of the file, which is group `users_group` in this example
* `371`: the number of data bytes in this file
* `Sep 4 2023`: the date the file was last modified
* `file.txt`: the name of the resource (in this case, the file, which is named `file.txt`)

## File permissions and security

Do note that user accounts are an operating system-enforced distinction and can therefore be bypassed by booting a different operating system on the machine (as is common with removable devices for installation and maintenance purposes). Physical access to a computer in and of itself is enough to constitute a significant security risk since physical access can bypass all user account protections.

## User permissions and data
Processes are generally launched as chidren of another process. When a user logs in, they can see processes including those that they launch as their own user. All processes run from a specific user session will therefore inherit that user's identity as the running user.

# The superuser

Operating systems generally have a special account or group that can run essentially unrestricted to bypass normal account rules. For POSIX and Unix-like systems, this account is called `root`, and may be referred to in other contexts as the "superuser" or "administrator" account.

Regular user accounts cannot switch user accounts directly. There are two caveats, however:
* The superuser/`root` can act as any user as it wishes
* Some user accounts can act as other users after going through OS-mediated authentication

Like with hardware-granted privilege, losing privilege is far easier than gaining privilege since gaining privilege requires a series of authentication measures and checks.

Some users create their own accounts to run directly as `root` or administrator. **_This is dangerous!_** Running directly as `root` gives you and your processes authority to ignore OS-enforced protections, which can lead to a very bad time very quickly.

Consider the following:
```sh
rm -rf /*
```

Running this as an unprivileged user (and without other authentication to become root) will raise errors and warnings locking you out of the action. Running this as `root` (or after authentication) will _deallocate your entire operating system_ from its disk space; in effect, running _this single command_ under the right authorization will erase your operating system without _any complaints or warnings_.

# Authentication outside the kernel


