# Computers and multitasking

Consider the problem of adding up the square root of $x$ numbers.

For a simple case, we can choose $x = 10$. Writing a for loop to add up the first ten numbers (1-indexed, so 1 to 10) naturally follows:

```c
double sum = 0.0;

for (int i = 1; i <= 10; i += 1) {
    sum += sqrt((double) i);
}
```

We can choose $x = 100$, $x = 1000$, or even $x = 10000$. Our solution won't really change. We might run a _little_ slower as we increase $x$, but the difference by itself won't be enough to notice.

What if we set $x = 10,000,000,000$? We can try the same approach:

```c
double sum = 0.0;

for (int i = 1; i <= 10000000000; i += 1) {
    sum += sqrt((double) i);
}
```

The trouble now is that this is significantly slower. For me, this took about _16 seconds_ to complete. Sure, this example is arbitrary, but we'd like computations on our computers generally, especially the most intense ones, to not take 16 seconds at a time. In fact, a wide majority of consumer laptops and desktops (and even certain smartphones) have carefully engineered processors designed to handle multiple tasks at once without slowing down. This reading, and the readings in this directory, are designed to briefly answer:
1. How can computers handle multiple _different_ tasks at once?
2. How can computers break up a large task into multiple smaller tasks which can be run at once?

For this problem, we can fortunately identify very helpful details[^1]. The problem is purely additive, so:
* We can group our additions into larger-sized "chunks" by the associative property: $$\sqrt{1} + \sqrt{2} + \sqrt{3} + \sqrt{4} = (\sqrt{1} + \sqrt{2}) + (\sqrt{3} + \sqrt{4})$$ and so on.
* We don't have to worry about the order in which we complete the additions by the commutative property: $$(\sqrt{1} + \sqrt{2}) + (\sqrt{3} + \sqrt{4}) = (\sqrt{3} + \sqrt{4}) + (\sqrt{1} + \sqrt{2})$$ and so on.

The loop over 10 billion numbers tacitly assumes that we only run the code in one instance (we only have one "agent", as I'll call it, that computes the result going one-by-one through the numbers). What if we had multiple instances being run by multiple "agents"? As a matter of fact, given the properties we identified about this problem, we can split the work of our problem very cleanly between several agents. In pseudocode, we can express this roughly as:

```
sum = 0.0	// shared by all agents. Take turns adding to this at the end.

for (0, number of agents) spawn an agent

agent 1 runs:
    local_sum = 0.0
    for [1, 2500000000] inclusive local_sum += sqrt(current_number)
    
    check if another agent is adding to sum; wait if true
    tell other agents "it's my turn"
    sum += local_sum
    tell other agents "all done"

agent 2 runs:
    same thing for [2500000001, 5000000000] inclusive

agent 3 runs:
    same thing for [5000000001, 7500000000] inclusive

agent 4 runs:
    same thing for [7500000001, 10000000000] inclusive
```

Approaching the problem _this_ way means we can shave off a good ten seconds and complete the sum in about 5 seconds total---a substantial improvement.

More formally, we are structuring the above code so that the agents run "in parallel": as much as possible, each agent doesn't depend on the work of other agents. The agents only depend on each other to agree upon and enforce _safe_[^2] addition to the shared sum. We have _parallelized_ our code (and _parallelized_ the problem generally).

Depending on the context, we can consider agents either as "threads" (which would be the case in this example) or "processes". There are some important distinctions between the two as we'll develop.

## Further reading for this example

See the full-fledged code [here](./multitask.c). I've documented this where I can, but would be happy to clarify more.

[^1]: What we develop here is not possible for _all_ problems. We might care about, for example, writing data to a file in a specific order, so situations where we want to write a large amount of data in order might require doing things serially unless we resort to ever-fancier optimizations (which are also ever-easier to mess up).
[^2]: Yes, you can throw caution to the wind, but it's ill-advised and you can get some nasty bugs. Perhaps even worse, these bugs are hard to replicate and therefore debug.

